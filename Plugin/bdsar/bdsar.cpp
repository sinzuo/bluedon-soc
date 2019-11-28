/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 * 
 * @file   tsar.cpp 
 * @brief   tsar
 * 获取系统cpu mem load等信息
 *
 * @version 1.0 
 * @author  范毅
 * @date    2016年11月11日
 *
 * 修订说明：最初版本
 **************************************************/
#include "bdsar.h"
#include "stdio.h"
#include <stdlib.h>
#include <sys/statfs.h>
#include <mntent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/un.h>
#include <netdb.h>
#include <dirent.h>
#include <fstream>
#include <sstream>


using namespace Poco;

#ifndef DEBUG
#define DEBUG 0
#endif

/**
 * 获取squid运行参数
 * @param       NULL
 * @return      0:success -1:failed
 */
int bdSarSquid::collect() {

    //const char *cmd = "cache_object://172.16.12.48/info";
    int port = getPort();
    string ip = getIP();
    string cmd = getCmd();
    if (cmd == "") {
        cmd = "cache_object://";
        cmd += ip;
        cmd += "/info";
    }

    string str = createHttpHead(cmd.c_str(), NULL,  ip.c_str(), port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }

    struct sockaddr_in servaddr;
    FILE* f = NULL;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
    int m;
    if ((m = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
        return -1;
    }
    char line[4096];
    memset(line, 0, sizeof(line));
    write (sockfd, str.c_str(), str.size()+1);
    pre_squid = cur_squid;
    f = fdopen(sockfd, "r");
    while(fgets(line, 4096, f) != NULL) {
        string str = line;
        if (str.find("Number of file desc currently in use:") != string::npos){
            sscanf(line, " Number of file desc currently in use:%u", &cur_squid.si.fu.fd_used);
        }
        else if (str.find("Files queued for open:") != string::npos) {
            sscanf(line, " Files queued for open:%u", &cur_squid.si.fu.fd_queue);
        }
        else if (str.find("StoreEntries with MemObjects") != string::npos) {
            sscanf(line, "%u StoreEntries with MemObjects", &cur_squid.si.iid.memobjs);
        }
        else if (str.find("StoreEntries") != string::npos) {
            sscanf(line, "%u StoreEntries", &cur_squid.si.iid.memobjs);
        }
        else if (str.find("Hot Object Cache Items") != string::npos) {
            sscanf(line, "%u", &cur_squid.si.iid.hotitems);
        }
        else if (str.find("Mean Object Size") != string::npos) {
            sscanf(line, "%llu", &cur_squid.si.sf.meanobjsize);
        }
        else if (str.find("Memory hits as % of hit requests:") != string::npos) {
            double x;
            sscanf(line, " Memory hits as %*s of hit requests:	5min: %*s 60min: %lf", &x);//&cur_squid.si.sf.mem_hit);
            cur_squid.si.sf.mem_hit = x * 10;
        }
        else if (str.find("Disk hits as % of hit requests:") != string::npos) {
            double x;
            sscanf(line, " Disk hits as %*s of hit requests:	5min: %*s 60min: %lf", &x);
            cur_squid.si.sf.disk_hit = x * 10;
        }
        else if (str.find("Hits as % of bytes sent:") != string::npos) {
            double x;
            sscanf(line + 1, "Hits as %*s of bytes sent:	5min: %*s 60min: %lf", &x);//&cur_squid.si.sf.mem_hit);
            cur_squid.si.sf.byte_hit_rate = x * 10;
        }
        else if (str.find("Hits as % of all requests:") != string::npos) {
            double x;
            sscanf(line + 1, "Hist as %*s of all requests:	5min: %*s 60min: %lf", &x);
            cur_squid.si.sf.http_hit_rate = x * 10;
        }
        else if (str.find("Average HTTP requests per minute since start:") != string::npos) {
            double x;
            sscanf(line, "	Average HTTP requests per minute since start:	%lf", &x);
            cur_squid.si.sf.responsetime = x * 100;
            cur_squid.si.sf.byte_hit_rate = x * 100;
        }
        else if (str.find("HTTP Requests (All):") != string::npos) {
            double x;
            sscanf(line, "	HTTP Requests (ALL):	%lf", &x);
            cur_squid.sc.cc.http_requests = x;
        }
    }

    close(sockfd);
    fclose(f);
    return 0;
}

string bdSarSquid::Output() {
    squid_data st_squid;
    stringstream stream;
    string str;

    if (cur_squid.sc.cc.http_requests >= pre_squid.sc.cc.http_requests) {
        st_squid.qps = (cur_squid.sc.cc.http_requests - pre_squid.sc.cc.http_requests) / _interval;
    }
    else {
        st_squid.qps = 0.0;
    }

    st_squid.rt = cur_squid.si.sf.responsetime / 100.0;
    st_squid.r_hit = cur_squid.si.sf.http_hit_rate / 10.0;
    st_squid.b_hit = cur_squid.si.sf.byte_hit_rate / 10.0;
    st_squid.d_hit = cur_squid.si.sf.disk_hit / 10.0;
    st_squid.m_hit = cur_squid.si.sf.mem_hit / 10.0;
    st_squid.fdused = cur_squid.si.fu.fd_used;
    st_squid.fdque = cur_squid.si.fu.fd_queue;
    st_squid.objs = cur_squid.si.iid.entries;
    st_squid.inmem = cur_squid.si.iid.memobjs;
    st_squid.hot = cur_squid.si.iid.hotitems;
    st_squid.size = (cur_squid.si.sf.meanobjsize << 10) / 100.0;
    //st_squid.totalp = _squid_nr;
    //st_squid.livep = _live_squid_nr;

    stream << st_squid.qps << "~"
           << st_squid.rt << "~"
           << st_squid.r_hit << "~"
           << st_squid.b_hit << "~"
           << st_squid.d_hit << "~"
           << st_squid.m_hit << "~"
           << st_squid.fdused << "~"
           << st_squid.fdque << "~"
           << st_squid.objs << "~"
           << st_squid.inmem << "~"
           << st_squid.hot << "~"
           << st_squid.size;// << "~"
           //<< st_squid.totalp << "~"
           //<< st_squid.livep;

    stream >> str;
    return str;
}


/*
 *************************************************************
 * Read swapping statistics from /proc/vmstat & /proc/meminfo.
 *************************************************************
 */

int bdSarApache::collect() {
    int sockfd, m, send;
    // int fd, n;
    char    line[LEN_4096] = {0};

    //char cmd[] = "Server-status?auto";

    struct sockaddr_in servaddr;
    FILE *stream = NULL;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        if (sockfd){
            close(sockfd);
        }
    }

    string ip = getIP();
    int port = getPort();
    string cmd = getCmd();
    string authinfo = getAuthInfo();

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);

    string request = createHttpHead(cmd.c_str(), authinfo.c_str(), ip.c_str(), port);

    if ((m = connect(sockfd, (struct sockaddr *) &servaddr,
                    sizeof(servaddr))) == -1 ) {
        if (sockfd) {
            close(sockfd);
        }
        return -1;
    }

    if ((send = write(sockfd, request.c_str(), request.size())) == -1) {
        //goto writebuf;
        if(stream) {
            if (fclose(stream) < 0) {
                if (sockfd) {
                    close(sockfd);
                }
            }
        }
        return NULL;
    }
    stream = fdopen(sockfd, "r");
    if (!stream) {
        if (sockfd) {
            close(sockfd);
        }
        return NULL;
    }
    pre_apache = cur_apache;
    while(fgets(line, LEN_4096, stream) != NULL) {

        log_debug("apache:%s", line);

        if(!strncmp(line, "Total kBytes:", 13)) {
            sscanf(line + 14, "%llu", &cur_apache.kBytes_sent);

        } else if (!strncmp(line, "BusyWorkers:", 12)) {
            sscanf(line + 13, "%d", &cur_apache.busy_proc);

        } else if (!strncmp(line, "IdleWorkers:", 12)) {
            sscanf(line + 13, "%d", &cur_apache.idle_proc);

        } else {
            ;
        }
        memset(line, 0, LEN_4096);
    }

    return 0;
}

string bdSarApache::Output() {
    apache_data st_apache;
    st_apache.reset();
    if (cur_apache.query < pre_apache.query
            || cur_apache.response_time < pre_apache.response_time
            || cur_apache.kBytes_sent < pre_apache.kBytes_sent){
        return "";
    }
    if (pre_apache.busy_proc == 0 && pre_apache.idle_proc == 0) {
        return "";
    }

    if (cur_apache.query >= pre_apache.query) {
        st_apache.qps = (cur_apache.query - pre_apache.query) / _interval * 1.0;
    }
    if ((cur_apache.response_time >= pre_apache.response_time) && (cur_apache.query > pre_apache.query)) {
        st_apache.rt = (cur_apache.response_time - pre_apache.response_time) / (cur_apache.query - pre_apache.query) / 1000 * 1.0;
    }
    if (cur_apache.kBytes_sent >= pre_apache.kBytes_sent) {
        log_debug("cur_apache.kBytes_sent:%d, pre_apache.kBytes_sent:%d", cur_apache.kBytes_sent, pre_apache.kBytes_sent);
        st_apache.sent = (cur_apache.kBytes_sent - pre_apache.kBytes_sent) * 100 / _interval * 1.0 / 100;
    }

    st_apache.busy = cur_apache.busy_proc * 1.0;
    st_apache.idle = cur_apache.idle_proc * 1.0;

    std::stringstream stream;
    stream << st_apache.qps << "~"
           << st_apache.rt << "~"
           << st_apache.sent << "~"
           << st_apache.busy << "~"
           << st_apache.idle;
    string buf;
    stream >> buf;

    return buf;
}


/*
 * start collect haproxy info
 * @return: 0 true, -1 failure
 */
int bdSarHaproxy::collect() {
    string ip = getIP();
    int port = getPort();

    pre_haproxy = cur_haproxy;
    cur_haproxy.reset();
    char line[LEN_1024];
    memset(line, 0, sizeof(line));

    //const char *cmd = "/haproxy-stats";
    string cmd = getCmd();
    string authinfo = getAuthInfo();
    string httpHead = createHttpHead(cmd.c_str(), authinfo.c_str(), getIP().c_str(), port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd <= 0 ) {
        log_error("haproxy create socket failed, please check!");
        return -1;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    FILE* f = NULL;
    inet_pton(AF_INET, getIP().c_str(), &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        close(sockfd);
        log_error("connect haproxy failed, please check!");
        return -1;
    }

    write (sockfd, httpHead.c_str(), httpHead.size()+1);

    f = fdopen(sockfd, "r");
    if (f == NULL) {
        close(sockfd);
        log_error("fdopen sockdfd failed, please check!");
        return -1;
    }

    cur_haproxy.stat = 0;

    while(fgets(line, LEN_1024, f) != NULL) {
        string sText(line);
        int size = 0;
        if ((string::npos != sText.find("200 OK")) > 0) {
            cur_haproxy.stat = 1;
        }
        else if ((string::npos != sText.find("uptime")) > 0) {
            int d,m,h,s;
            sscanf(sText.c_str(), "<b>uptime = </b>%dd%dh%dm%ds*", &d, &h, &m, &s);
            cur_haproxy.uptime = d * 24 * 60 * 60 + h * 60 * 60 + m * 60 + s;
        }
        else if ((string::npos != sText.find("current conns")) >= 0) {
            cur_haproxy.conns = 1;
            sscanf(sText.c_str(), "current conns = %lu;*", &cur_haproxy.conns);
        }
        else if ((string::npos != sText.find("Current request rate")) >= 0) {

            sscanf(sText.substr(size).c_str(), "Current request rate:</th><td>%lu/s</td>*", &cur_haproxy.qps);
        }
    }

    if (cur_haproxy.stat != 1) {
        log_warn("haproxy stat abnormal!");
    }
    close(sockfd);
    fclose(f);
    return 0;
}

string bdSarBase::createHttpHead(const char *cmd, const char *authinfo, const char *ip, int port){

    if (cmd == NULL || ip == NULL) {
        return "";
    }
    string head, str;
    stringstream stream;
    stream << port;
    stream >> str;
    head = "GET ";
    head += cmd;
    head += " HTTP/1.1\r\nAccept: */*\r\nUser-Agent: Mozilla/6.0\r\nHost: ";
    head += ip;
    head += ":";
    head += str;
    if (authinfo != NULL) {
        head += "\r\nAuthorization: Basic ";
        head += authinfo;
    }
    head += "\r\nAccept-Language: cn\r\nConnection: Close\r\n\r\n";
    //log_debug("the http head is : %s", head.c_str());
    return head;
}

string bdSarHaproxy::Output() {
    haproxy_data st_haproxy;
    st_haproxy.stat = cur_haproxy.stat;
    st_haproxy.uptime = cur_haproxy.uptime;
    st_haproxy.conns = cur_haproxy.conns;
    if (cur_haproxy.qps > pre_haproxy.qps){
        st_haproxy.qps = cur_haproxy.qps - pre_haproxy.qps;
    }
    st_haproxy.hit = cur_haproxy.hit / 10 * 1.0;
    st_haproxy.rt = cur_haproxy.rt / 100 * 1.0;

    std::stringstream stream;
    string buf;
    stream << st_haproxy.stat << "~"
           << st_haproxy.uptime << "~"
           << st_haproxy.conns << "~"
           << st_haproxy.qps << "~"
           << st_haproxy.hit << "~"
           << st_haproxy.rt;
    stream >> buf;
    return buf;
}

/*
 * collect nginx info
 * @return: 0 true 1 failure 
 */
int bdSarNginx::collect(){
    int                 m, sockfd, send;
    void               *addr;
    char               line[LEN_4096];
    string             request;
    FILE               *stream = NULL;

    struct sockaddr_in  servaddr;

    pre_nginx = cur_nginx;
    cur_nginx.reset();

    int port = getPort();
    string ip = getIP();
    string cmd = getCmd();
    string authinfo = getAuthInfo();

    addr = &servaddr;
    bzero(addr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        log_error("collecting nginx data, create socket failed, please check!");

        return -1;
    }

    //http head
    request = createHttpHead(cmd.c_str(), authinfo.c_str(), ip.c_str(), port);

    if ((m = connect(sockfd, (struct sockaddr *) addr, sizeof(servaddr))) == -1 ) {

        log_error("collecting nginx data, connect failed, please check!");

        return -1;
    }

    if ((send = write(sockfd, request.c_str(), request.size())) == -1) {

        log_error("collecting nginx data, write failed, please check!");

        return -1;
    }

    if ((stream = fdopen(sockfd, "r")) == NULL) {

        log_error("collecting nginx data, fdopen failed, please check!");

        return -1;
    }

    /*for nginx*/
    while (fgets(line, LEN_4096, stream) != NULL) {

        log_debug("line:%s", line);

        if (!strncmp(line, "Active connections:", sizeof("Active connections:") - 1)) {

            sscanf(line + sizeof("Active connections:"), "%llu", &cur_nginx.nactive);

        } else if (!strncmp(line, 
                            "server accepts handled requests",
                            sizeof("server accepts handled requests") - 1)
                  ) {

            if (fgets(line, LEN_4096, stream) != NULL) {
                if (!strncmp(line, " ", 1)) {
                    sscanf(line + 1, "%llu %llu %llu",
                            &cur_nginx.naccept, &cur_nginx.nhandled, &cur_nginx.nrequest);
                }
            }

        } else if (!strncmp(line, "Server accepts:", sizeof("Server accepts:") - 1)) {

            sscanf(line , "Server accepts: %llu handled: %llu requests: %llu request_time: %llu",
                    &cur_nginx.naccept, &cur_nginx.nhandled, &cur_nginx.nrequest, &cur_nginx.nrstime);

        } else if (!strncmp(line, "Reading:", sizeof("Reading:") - 1)) {

            sscanf(line, "Reading: %llu Writing: %llu Waiting: %llu",
                    &cur_nginx.nreading, &cur_nginx.nwriting, &cur_nginx.nwaiting);

        } else if (!strncmp(line, "SSL:", sizeof("SSL:") - 1)) {

            sscanf(line, "SSL: %llu SPDY: %llu",
                    &cur_nginx.nssl, &cur_nginx.nspdy);

        } else if (!strncmp(line, "SSL_Requests:", sizeof("SSL_Requests:") - 1)) {

            sscanf(line, "SSL_Requests: %llu SSL_Handshake: %llu SSL_Handshake_Time: %llu",
                    &cur_nginx.nssl, &cur_nginx.nsslhsc, &cur_nginx.nsslhst);

        } else {

            continue;

        }
    }

    return 0;
}

/*
 * nginx info output
 * @return: nginx info string
 */
string bdSarNginx::Output() {

    nginx_data st_nginx;
    st_nginx.reset();
    unsigned long long *pre_data = &pre_nginx.naccept;
    unsigned long long *cur_data = &cur_nginx.naccept;
    double *st_data = &st_nginx.accept;

    for (int i = 0; i < 3; i++) {
        if (*cur_data >= *pre_data) {
            *st_data = *cur_data - *pre_data;
        }
        st_data++;
        cur_data++;
        pre_data++;
    }

    for (int i = 3; i < 7; i++) {
        *st_data++ = *cur_data++;
    }

    //wait
    if (cur_nginx.nrequest >= pre_nginx.nrequest) {
        st_nginx.qps = (cur_nginx.nrequest - pre_nginx.nrequest) * 1.0 / _interval;
    }

    if (cur_nginx.nrstime >= pre_nginx.nrstime) {
        if (cur_nginx.nrequest > pre_nginx.nrequest) {
            st_nginx.rt = (cur_nginx.nrstime - pre_nginx.nrstime) * 1.0 / (cur_nginx.nrequest - pre_nginx.nrequest);
        }
    }

    if (cur_nginx.nssl >= pre_nginx.nssl) {
        st_nginx.sslqps = (cur_nginx.nssl - pre_nginx.nssl) * 1.0 / _interval;
    }

    if (cur_nginx.nspdy > pre_nginx.nspdy) {
        st_nginx.spdyps = (cur_nginx.nspdy - pre_nginx.nspdy) * 1.0 / _interval;
    }

    if (cur_nginx.nsslhst >= pre_nginx.nsslhst) {
        if (cur_nginx.nsslhsc > pre_nginx.nsslhsc) {
            st_nginx.sslhst = (cur_nginx.nsslhst - pre_nginx.nsslhst) * 1.0 / (cur_nginx.nsslhsc - pre_nginx.nsslhsc);
        }
    }

    std::stringstream stream;
    string buf;
    stream << st_nginx.accept << "~"
           << st_nginx.handle << "~"
           << st_nginx.reqs << "~"
           << st_nginx.active << "~"
           << st_nginx.read << "~"
           << st_nginx.write << "~"
           << st_nginx.wait << "~"
           << st_nginx.qps << "~"
           << st_nginx.rt << "~"
           << st_nginx.sslqps << "~"
           << st_nginx.spdyps << "~"
           << st_nginx.sslhst;

    stream >> buf;

    return buf;
}

/*
 * 解析nginx参数信息
 */
void bdSarNginx::cal_nginx_info(PIN stats_nginx pre_nginx, PIN stats_nginx cur_nginx, POUT nginx_data &st_nginx){

    unsigned long long *pre_data = &pre_nginx.naccept;
    unsigned long long *cur_data = &cur_nginx.naccept;
    double *st_data = &st_nginx.accept;

    for (int i = 0; i < 3; i++) {
        if (*cur_data >= *pre_data) {
            *st_data = *cur_data - *pre_data;
        }
        st_data++;
        cur_data++;
        pre_data++;
    }

    for (int i = 3; i < 7; i++) {
        *st_data++ = *cur_data++;
    }

    //wait
    if (cur_nginx.nrequest >= pre_nginx.nrequest) {
        st_nginx.qps = (cur_nginx.nrequest - pre_nginx.nrequest) * 1.0 / _interval;
    }

    if (cur_nginx.nrstime >= pre_nginx.nrstime) {
        if (cur_nginx.nrequest > pre_nginx.nrequest) {
            st_nginx.rt = (cur_nginx.nrstime - pre_nginx.nrstime) * 1.0 / (cur_nginx.nrequest - pre_nginx.nrequest);
        }
    }

    if (cur_nginx.nssl >= pre_nginx.nssl) {
        st_nginx.sslqps = (cur_nginx.nssl - pre_nginx.nssl) * 1.0 / _interval;
    }

    if (cur_nginx.nspdy > pre_nginx.nspdy) {
        st_nginx.spdyps = (cur_nginx.nspdy - pre_nginx.nspdy) * 1.0 / _interval;
    }

    if (cur_nginx.nsslhst >= pre_nginx.nsslhst) {
        if (cur_nginx.nsslhsc > pre_nginx.nsslhsc) {
            st_nginx.sslhst = (cur_nginx.nsslhst - pre_nginx.nsslhst) * 1.0 / (cur_nginx.nsslhsc - pre_nginx.nsslhsc);
        }
    }
}

/**
 * create module
 * @param       NULL
 * @return      bdSarBase
 */
bdSarBase *bdSarFactory::create(const char* module) {
    if (!strcmp(module, "nginx")) {
        return new bdSarNginx;
    }
    else if (!strcmp(module, "apache")) {
        return new bdSarApache;
    }
    else if (!strcmp(module, "haproxy")) {
        return new bdSarHaproxy;
    }
    else if (!strcmp(module, "squid")) {
        return new bdSarSquid;
    }
    else {
        return NULL;
    }
}

#ifndef __BD_SAR_H__
#define __BD_SAR_H__
/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file   bdsar.h
 * @brief   bdsar
 * 获取系统cpu mem load等信息
 *
 * @version 1.0 
 * @author  fanyi
 * @date    2016年11月11日
 *
 * 修订说明：最初版本
 **************************************************/
#include <iostream>
#include <linux/major.h>
#include <list>
#include <vector>
#include <iomanip>
#include <sys/socket.h>
#include "BdSarMgr.h"

using namespace std;

#ifndef PIN
#define PIN
#endif

#ifndef POUT
#define POUT
#endif

#ifndef PINOUT
#define PINOUT
#endif


#include <string.h>
#include <string>


#define LEN_128     128
#define LEN_512     512
#define LEN_1024    1024
#define LEN_4096    4096

struct stats_apache {
    unsigned int        busy_proc;
    unsigned int        idle_proc;
    unsigned long long  query;
    unsigned long long  response_time;
    unsigned long long  kBytes_sent;

    void reset() {
        busy_proc = 0;
        idle_proc = 0;
        query = 0;
        response_time = 0;
        kBytes_sent = 0;
    }

    void print() {
        log_debug("==========================  print apache info begin  =================================");
        log_debug("busy_proc:%lu", busy_proc);
        log_debug("idle_proc:%lu", idle_proc);
        log_debug("query:%lu", query);
        log_debug("response_time:%lu", response_time);
        log_debug("kBytes_sent:%lu", kBytes_sent);
        log_debug("==========================   print apache info end   =================================");
    }
};

struct apache_data{
    double qps;
    double rt;
    double sent;
    double busy;
    double idle;

    void reset() {
        qps = 0;
        rt = 0;
        sent = 0;
        busy = 0;
        idle = 0;
    }

    void print() {
        log_debug("==========================  print apache cal begin  =================================");
        log_debug("qps:%lu", qps);
        log_debug("rt:%lu", rt);
        log_debug("sent:%lu", sent);
        log_debug("busy:%lu", busy);
        log_debug("idle:%lu", idle);
        log_debug("==========================   print apache cal end   =================================");
    }
};

struct stats_haproxy {
    unsigned long stat;
    unsigned long uptime;
    unsigned long conns;
    unsigned long qps;
    unsigned long hit;
    unsigned long rt;

    void reset() {
        stat = 0;
        uptime = 0;
        conns = 0;
        qps = 0;
        hit = 0;
        rt = 0;
    }

    void print() {
        std::cout << "==========================  print haproxy info begin  =================================" << std::endl
                  << "stat:" << stat << std::endl
                  << "uptime:" << uptime << std::endl
                  << "conns:" << conns << std::endl
                  << "qps:" << qps << std::endl
                  << "hit:" << hit << std::endl
                  << "rt:" << rt << std::endl
                  << "==========================   print haproxy info end   =================================" << std::endl;
    }
};

struct haproxy_data {
    double stat;
    double uptime;
    double conns;
    double qps;
    double hit;
    double rt;

    void reset() {
        stat = 0;
        uptime = 0;
        conns = 0;
        qps = 0;
        hit = 0;
        rt = 0;
    }

    void print() {
        std::cout << "==========================  print haproxy info begin  =================================" << std::endl
                  << "stat:" << stat << std::endl
                  << "uptime:" << uptime << std::endl
                  << "conns:" << conns << std::endl
                  << "qps:" << qps << std::endl
                  << "hit:" << hit << std::endl
                  << "rt:" << rt << std::endl
                  << "==========================   print haproxy info end   =================================" << std::endl;
    }
};

struct stats_nginx {
    unsigned long long naccept;     /* accepted connections */
    unsigned long long nhandled;    /* handled connections */
    unsigned long long nrequest;    /* handled requests */
    unsigned long long nactive;     /* number of all open connections including connections to backends */
    unsigned long long nreading;    /* nginx reads request header */
    unsigned long long nwriting;    /* nginx reads request body, processes request, or writes response to a client */
    unsigned long long nwaiting;    /* keep-alive connections, actually it is active - (reading + writing) */
    unsigned long long nrstime;     /* reponse time of handled requests */
    unsigned long long nspdy;       /* spdy requests */
    unsigned long long nssl;        /* ssl requests */
    unsigned long long nsslhst;     /* ssl handshake time*/
    unsigned long long nsslhsc;     /* ssl handshake count*/

    void reset() {
        naccept = 0;     /* accepted connections */
        nhandled = 0;    /* handled connections */
        nrequest = 0;    /* handled requests */
        nactive = 0;     /* number of all open connections including connections to backends */
        nreading = 0;    /* nginx reads request header */
        nwriting = 0;    /* nginx reads request body, processes request, or writes response to a client */
        nwaiting = 0;    /* keep-alive connections, actually it is active - (reading + writing) */
        nrstime = 0;     /* reponse time of handled requests */
        nspdy = 0;       /* spdy requests */
        nssl = 0;        /* ssl requests */
        nsslhst = 0;     /* ssl handshake time*/
        nsslhsc = 0;     /* ssl handshake count*/
    }

    void print() {
        std::cout << "==========================  print haproxy info begin  =================================" << std::endl
                  << "naccept:" << naccept << std::endl
                  << "nhandled:" << nhandled << std::endl
                  << "nrequest:" << nrequest << std::endl
                  << "nactive:" << nactive << std::endl
                  << "nreading:" << nreading << std::endl
                  << "nwriting:" << nwriting << std::endl
                  << "nwaiting:" << nwaiting << std::endl
                  << "nrstime:" << nrstime << std::endl
                  << "nspdy:" << nspdy << std::endl
                  << "nssl:" << nssl << std::endl
                  << "nsslhst:" << nsslhst << std::endl
                  << "nsslhsc:" << nsslhsc << std::endl
                  << "==========================   print haproxy info end   =================================" << std::endl;
    }
};

struct nginx_data{
    double accept;
    double handle;
    double reqs;
    double active;
    double read;
    double write;
    double wait;
    double qps;
    double rt;
    double sslqps;
    double spdyps;
    double sslhst;

    void reset(){
        accept = 0;
        handle = 0;
        reqs = 0;
        active = 0;
        read = 0;
        write = 0;
        wait = 0;
        qps = 0;
        rt = 0;
        sslqps = 0;
        spdyps = 0;
        sslhst = 0;
    }

    void print() {
        std::cout << "==========================  print haproxy info begin  =================================" << std::endl
                  << "accept:" << accept << std::endl
                  << "handle:" << handle << std::endl
                  << "reqs:" << reqs << std::endl
                  << "active:" << active << std::endl
                  << "read:" << read << std::endl
                  << "write:" << write << std::endl
                  << "wait:" << wait << std::endl
                  << "pqs:" << qps << std::endl
                  << "rt:" << rt << std::endl
                  << "sslpqs:" << sslqps << std::endl
                  << "spdyps:" << spdyps << std::endl
                  << "sslhst:" << sslhst << std::endl
                  << "==========================   print haproxy info end   =================================" << std::endl;
    }
};

struct squid_counters
{
    struct counters_client {
        unsigned long long http_requests;
        unsigned long long http_hits;
        unsigned long long http_kbytes_out;
        unsigned long long http_hit_kbytes_out;

    } cc;
    struct counters_server {
        unsigned long long all_requests;
        unsigned long long all_kbytes_in;
        unsigned long long all_kbytes_out;
    } cs;
};
struct squid_info {
    void print() {
        mu.print();
        fu.print();
        iid.print();
        sf.print();

    }
    struct mem_usage {
        unsigned long long mem_total;
        unsigned long long mem_free;
        unsigned long long mem_size;

        void print() {
            std::cout << "mem_usage begin:" << std::endl
                      << "mem_total:" << mem_total << std::endl
                      << "mem_free:" << mem_free << std::endl
                      << "mem_size:" << mem_size << std::endl
                      << "mem_usage end" << std::endl;
        }
    } mu;
    struct fd_usage {
        unsigned int fd_used;
        unsigned int fd_queue;
        void print() {
            std::cout << "fd_usage begin:" << std::endl
                      << "fd_used:" << fd_used << std::endl
                      << "fd_queue:" << fd_queue << std::endl
                      << "mem_usage end" << std::endl;
        }
    } fu;
    struct info_internal_data {
        unsigned int entries;
        unsigned int memobjs;
        unsigned int hotitems;

        void print() {
            std::cout << "info_internal_data begin:" << std::endl
                      << "entries:" << entries << std::endl
                      << "memobjs:" << memobjs << std::endl
                      << "hotitems:" << hotitems << std::endl
                      << "info_internal_data end" << std::endl;
        }
    } iid;
    struct squid_float {
        unsigned long long meanobjsize;
        unsigned long long responsetime;
        unsigned long long disk_hit;
        unsigned long long mem_hit;
        unsigned long long http_hit_rate;
        unsigned long long byte_hit_rate;

        void print() {
            std::cout << "squid_float begin:" << std::endl
                      << "meanobjsize:" << meanobjsize << std::endl
                      << "responsetime:" << responsetime << std::endl
                      << "disk_hit:" << disk_hit << std::endl
                      << "mem_hit:" << mem_hit << std::endl
                      << "http_hit_rate:" << http_hit_rate << std::endl
                      << "byte_hit_rate:" << byte_hit_rate << std::endl
                      << "squid_float end" << std::endl;
        }
    } sf;
};

struct stats_squid {
    bool usable;
    struct squid_counters sc;
    struct squid_info si;

    void reset() {
        usable = false;
        memset(&sc, 0, sizeof(sc));
        memset(&si, 0, sizeof(si));
    }

    void print() {
        std::cout << "==========================  print haproxy info begin  =================================" << std::endl
                  << "usable:" << usable << std::endl;
        //sc.print();
        si.print();
        std::cout<< "==========================   print haproxy info end   =================================" << std::endl;
    }
};

struct p_squid_info {
    struct squid_counters *scp;
    struct squid_info * sip;
};
struct squid_data {
    double qps;
    double rt;
    double r_hit;
    double b_hit;
    double d_hit;
    double m_hit;
    double fdused;
    double fdque;
    double objs;
    double inmem;
    double hot;
    double size;
    double totalp;
    double livep;

    void reset() {
        qps = 0.0;
        rt = 0.0;
        r_hit = 0.0;
        b_hit = 0.0;
        d_hit = 0.0;
        m_hit = 0.0;
        fdused = 0.0;
        fdque = 0.0;
        objs = 0.0;
        inmem = 0.0;
        hot = 0.0;
        size = 0.0;
        totalp = 0.0;
        livep = 0.0;
    }
};

class bdSarBase{
public:
    bdSarBase() {
        _interval   = 300; // 5minutes
        _filename   = "";
        _modName    = "";
        _ip         = "";
        _port       = 80;  //默认使用80
        _policyItem = 0;
    }
    virtual int collect() = 0;
    virtual void initOption() {
    }

    virtual string Output(){ // = 0;
        return "";
    }

    void setInterval(int interval) {
        _interval = interval;
    }

    int getInterval() {
        return _interval;
    }

    void setIP(string ip) {
        _ip = ip;
    }

    string getIP(){
        return _ip;
    }

    void setPort(int port) {
        _port = port;
    }

    int getPort() {
        return _port;
    }

    void setModName(string name) {
        _modName = name;
    }

    string getModName() {
        return _modName;
    }

    int getItem() {
        return _policyItem;
    }

    void setItem(int policyItem) {
        _policyItem = policyItem;
    }

    void setAuthInfo(const char* authinfo) {
        _authinfo = authinfo;
    }

    void setCmd(const char* cmd) {
        _cmd = cmd;
    }

    string getCmd() {
        return _cmd;
    }

    string getAuthInfo() const { 
        return _authinfo;
    }

    string createHttpHead(const char *cmd, const char *authinfo, const char *ip, int port);
public:
    //时间间隔
    int _interval;
    string _filename;
private:
    string _ip;
    int    _port;
    string _modName;
    int _policyItem;
    string _authinfo;
    string _cmd;
};

class bdSarApache:public bdSarBase{
public:
    bdSarApache() {
        cur_apache.reset();
        setModName("apache");
    }
    ~bdSarApache() {
    }

    int collect();
    string Output();
private:
    stats_apache cur_apache;
    stats_apache pre_apache;
};

class bdSarHaproxy:public bdSarBase{
public:
    bdSarHaproxy() {
        cur_haproxy.reset();
        pre_haproxy.reset();
        setModName("haproxy");
    }
    ~bdSarHaproxy() {
    }

    int collect();
    string Output();

private:
    stats_haproxy cur_haproxy;
    stats_haproxy pre_haproxy;
};

class bdSarNginx:public bdSarBase{
public:
    bdSarNginx() {
        setModName("nginx");
    }
    ~bdSarNginx() {
    }

    int collect();
    string Output();
private:
    void init_nginx_host_info(struct nhostinfo *p);
    void cal_nginx_info(PIN stats_nginx pre_nginx, PIN stats_nginx cur_nginx, POUT nginx_data &st_nginx);

private:
    stats_nginx cur_nginx;
    stats_nginx pre_nginx;
};

class bdSarSquid:public bdSarBase{
public:
    bdSarSquid() {
        setModName("squid");
    }
    ~bdSarSquid() {
    }

    int collect();
    string Output();

private:
    void count_squid_nr() ;
    int __read_squid_stat(std::vector<stats_squid> &st_squids, int port, int index);
    int __get_squid_info(std::vector<stats_squid> &st_squids, const char *squidoption, char *squidcmd, int port, int index);
    int client_comm_connect(int sock, const char *dest_host, u_short dest_port, struct timeval *tvp);
    int parse_squid_info(char *buf, const char *cmd, struct p_squid_info *p_si);
    void collect_cnts(char *l, struct squid_counters *sc);
    void collect_info(char *l, struct squid_info *si) ;
    int read_a_int_value(char *buf, char *key, unsigned int *ret, int type);

    int read_a_float_value(char *buf, char *key, unsigned long long *ret, int type, int len) ;

    int read_a_long_long_value_squid(char *buf, char *key, unsigned long long *ret, int type) ;

    int read_a_long_long_value(char *buf, char *key, unsigned long long *ret, int type) ;

    ssize_t mywrite(int fd, void *buf, size_t len) {
        return send(fd, buf, len, 0);
    }

    ssize_t myread(int fd, void *buf, size_t len) {
        return recv(fd, buf, len, 0);
    }

private:
    stats_squid cur_squid;
    stats_squid pre_squid;
    //int _squid_nr;
    //int _port_list[MAXSQUID];
    //int _live_squid_nr;
};


/*
 * factory module for create module:nginx, haproxy, apache ..
 */
class bdSarFactory {
public:
    static bdSarBase* create(const char* module);
};
#endif //__BD_SAR_H__

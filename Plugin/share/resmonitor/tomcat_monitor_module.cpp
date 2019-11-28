#include "tomcat_monitor_module.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

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

#include "tinyxml.h"
#include "xpath_static.h"


const string VOID_ELEMENTS = ",br,hr,img,input,meta,link,param,area,base,col,source,embed,frame,keygen,";
inline void concatstr(const char *xstr, const char *fieldstr, string &obj)
{
    if(!xstr || !fieldstr) return;
    obj += "\"";
    obj += xstr;
    obj += "\"";
    obj += fieldstr;
}
string raradigm(const char *resource, string *array, int num)
{
    string tmp = resource;
    size_t pos = 0;
    for(int i = 0; i < num; i++)
    {
        string::size_type idx = tmp.find(array[i], pos);
        if(idx == string::npos)
        {
            cout << "operate value error...\n";
            break;
        }
        tmp[idx] = ',';
        tmp.insert(idx++, "\""); //给json字符串要加双引号
        tmp.insert(++idx, "\"");
        pos = idx + array[i].length() + 1;
    }

    tmp.insert(0, "\"");
    tmp.append("\"");
    pos = 0;
    while(true)
    {
        string::size_type idx = tmp.find(":", pos);
        if(idx == string::npos) break;
        tmp.insert(idx++, "\"");
        pos = idx + 1;
        if(pos >= tmp.length()) break;
        if(tmp[pos] == ' ')
            tmp[pos] = '\"';
        else
            tmp.insert(pos++, "\"");
    }

    return tmp;
}


TomcatMonitorModule::TomcatMonitorModule()
{
    setModName(string("tomcat"));

    initParameters();
}

TomcatMonitorModule::TomcatMonitorModule(const string &ip, int port, const string &cmd, const string &authinfo)
{
    setModName(string("tomcat"));
    setIP(ip);
    setPort(port);
    setCmd(cmd);
    setAuthInfo(authinfo);

    initParameters();
}

int TomcatMonitorModule::collect()
{
    int sockfd, res;
    int retvalue = 0;
    char tmpbuf[LEN_4096] = {0};

    //char cmd[] = "Server-status?auto";
    string content;
    struct sockaddr_in servaddr;
    FILE *stream = NULL;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        if(sockfd) close(sockfd);
        _errcode = -1;
        _errdesc = "Socket init failed";
        return -1;
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

    res = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if(res == -1)
    {
        retvalue = -1;
        _errcode = -1;
        _errdesc = "Connect remote ip failed";
        goto END;
    }

    res = write(sockfd, request.c_str(), request.size());
    if(res == -1)
    {
        retvalue = -1;
        _errcode = -1;
        _errdesc = "Write data failed";
        goto END;
    }
    stream = fdopen(sockfd, "r");
    if (!stream)
    {
        retvalue = -1;
        _errcode = -1;
        _errdesc = "Open sockfd failed";
        goto END;
    }

    do {
        memset(tmpbuf, 0, sizeof(tmpbuf));
        if(!fgets(tmpbuf, sizeof(tmpbuf), stream))
            break;
        string line = tmpbuf;
        if(line.length() > 2000)
            line.replace(line.find("<br>"), 4, " "); //去除<p></p>之间的空格
        do {
            string::size_type idx = line.find("nowrap>"); //去除没有值的属性nowrap
            if(idx == string::npos)
                break;
            line.replace(idx, sizeof("nowrap"), ">");
        }while(true);
        content += line;
    } while(true);


    if (res = parserHtml(content))
    {
        retvalue = -2;
        goto END;
    }

END:
    if(stream) //关闭文件描述符
    {
        if (fclose(stream) < 0)
        {
            if (sockfd)
                close(sockfd);
        }
    } else if(sockfd)
    {
        close(sockfd);
    }

    return retvalue;
}

string TomcatMonitorModule::Output()
{
    char buf[512];
    if(_errcode != 0)
    {
        sprintf(buf, "\"Error\":\"%s\"", _errdesc.c_str());
        return string(buf);
    }

    string res = "{"; //主括号开始
    res += "\"基本信息\":{";
    res += m_basicinfo + "}";
    res += ", ";
    res += "\"线程指标\":{";
    res += m_threadinfo + "}";
    res += ", ";
    res += "\"JVM指标\":{";
    res += m_jvminfo + "}";
    res += ", ";
    res += "\"应用系统指标\":{";
    res += m_operainfo + "}";
    res += "}";      //主括号结束

    return res;
}

int TomcatMonitorModule::parserHtml(const string &source)
{
    if(source.empty())
    {
        _errcode = -2;
        _errdesc = "Recieve data is empty";
        return -1;
    }
    string::size_type pos = source.find("\r\n\r\n");
    string content;
    if(pos != string::npos)
        content = source.substr(pos + 4);
    content = correctToXML(content);
//    cout << "----content:[\n" << content << "\n]\n";
//    content = "<html>"
//            "<head><title>504 Gateway Time-out</title></head>"
//            "<body bgcolor=\"white\">"
//            "<center><h1>504 Gateway Time-out</h1></center>"
//            "<hr /><center>BDWEB/1.9.9</center>"
//            "</body>"
//            "</html>";

    TiXmlDocument * XDp_doc = new TiXmlDocument();
    if(XDp_doc->Parse (content.c_str()))
    {
        delete XDp_doc;
        _errcode = -2;
        _errdesc = "XDp_doc->Parse error!";
        return -1;
    }
    TiXmlElement * XEp_main = XDp_doc->RootElement();
    TIXML_STRING S_res;
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[2]/td[1]/small/text()");//tomcat信息
    concatstr(S_res.c_str(), ":", m_basicinfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[3]/td[1]/small/text()");
    concatstr(S_res.c_str(), ", ", m_basicinfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[2]/td[2]/small/text()");//jvm信息
    concatstr(S_res.c_str(), ":", m_basicinfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[3]/td[2]/small/text()");
    concatstr(S_res.c_str(), "", m_basicinfo);
    cout << "m_basicinfo: " << m_basicinfo << endl;

    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[2]/td[4]/small/text()");//操作系统名称
    concatstr(S_res.c_str(), ":", m_operainfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[3]/td[4]/small/text()");
    concatstr(S_res.c_str(), ", ", m_operainfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[2]/td[5]/small/text()");//操作系统版本
    concatstr(S_res.c_str(), ":", m_operainfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[3]/td[5]/small/text()");
    concatstr(S_res.c_str(), ", ", m_operainfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[2]/td[6]/small/text()");//操作系统架构
    concatstr(S_res.c_str(), ":", m_operainfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/table[4]//tr[3]/td[6]/small/text()");
    concatstr(S_res.c_str(), "", m_operainfo);
    cout << "m_operainfo: " << m_operainfo << endl;

    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/p[1]/text()");//jvm指标
    string jvmsplits[] = {" Total", " Max"};
    m_jvminfo += raradigm(S_res.c_str(), jvmsplits, sizeof(jvmsplits)/sizeof(jvmsplits[0]));
    cout << "m_jvminfo: " << m_jvminfo << endl;

    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/p[2]/text()");//线程指标
    string threadsplits[] = {" Current", " Current", " Max", " Processing", " Request", " Error", " Bytes ", " Bytes"};
    m_threadinfo += raradigm(S_res.c_str(), threadsplits, sizeof(threadsplits)/sizeof(threadsplits[0]));
    cout << "m_threadinfo: " << m_threadinfo << endl;

    delete XDp_doc;

    return 0;
}

void TomcatMonitorModule::initParameters()
{
    m_basicinfo     = "";
    m_threadinfo    = "";
    m_jvminfo       = "";
    m_operainfo     = "";
}

//html转换为标准xml格式
string TomcatMonitorModule::correctToXML(const string &content)
{
    if(content.empty()) return "";

    string::size_type idxleft = content.find('<', 0);
    string::size_type idxright = 0;
    if(idxleft == string::npos) return content;

    string res;
    while(idxleft != string::npos)
    {
        res += content.substr(idxright, idxleft - idxright);
        idxright = content.find('>', idxleft + 1);
        string tmp = content.substr(idxleft + 1, idxright - idxleft - 1);
        if(tmp.find("</") == string::npos && tmp.find("/>") == string::npos)
        {
            string tag = tmp;
            string::size_type m = tmp.find(' ');
            if(m != string::npos)
                tag = tmp.substr(0, m);

            if(VOID_ELEMENTS.find(',' + tag + ',') != string::npos)
                tmp += " /"; //规范没有闭合的单标签
        }
        res += '<';
        res += tmp;
        res += '>';

        idxleft = content.find('<', ++idxright);
    }
    if(idxright < content.length())
        res += content.substr(idxright);

    return res;
}

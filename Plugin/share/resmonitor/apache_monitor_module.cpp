#include "apache_monitor_module.h"

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

inline void concatstr(const char *xstr, const char *fieldstr, string &obj)
{
    if(!xstr) return;
    obj += fieldstr;

    string tmp= "\"";
    tmp += xstr;
    tmp +=  "\"";

    string::size_type idx = tmp.find(":");
    if(idx == string::npos) return;
    tmp.insert(idx++, "\"");
    idx += 2;
    tmp.insert(idx++, "\"");

    obj += tmp;
}

ApacheMonitorModule::ApacheMonitorModule()
{
    setModName(string("apache"));

    initParameters();
}

ApacheMonitorModule::ApacheMonitorModule(const string &ip, int port, const string &cmd, const string &authinfo)
{
    setModName(string("apache"));
    setIP(ip);
    setPort(port);
    setCmd(cmd);
    setAuthInfo(authinfo);

    initParameters();
}

int ApacheMonitorModule::collect()
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

string ApacheMonitorModule::Output()
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
    res += "\"状态指标\":{";
    res += m_statusinfo + "}";
    res += "}";      //主括号结束

    return res;
}

void ApacheMonitorModule::initParameters()
{
    m_basicinfo = "";
    m_statusinfo = "";
}

int ApacheMonitorModule::parserHtml(const string &source)
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

    pos = content.find("<table");
    content = content.substr(0, pos - 2 );
    content += "</body>\r\n</html>";
//    cout << "----content:[\n" << content << "\n]\n";

    TiXmlDocument * XDp_doc = new TiXmlDocument();
    if(XDp_doc->Parse (content.c_str()))
    {
        delete XDp_doc;
        _errcode = -2;
        _errdesc = "XDp_doc->Parse error!";
        cout << "XDp_doc->Parse error!\n";
        return -1;
    }
    TiXmlElement * XEp_main = XDp_doc->RootElement();
    TIXML_STRING S_res;
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/dl[1]/dt[1]/text()");
    concatstr(S_res.c_str(), "", m_basicinfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/dl[1]/dt[2]/text()");
    concatstr(S_res.c_str(), ",", m_basicinfo);

    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/dl[2]/dt[3]/text()");
    concatstr(S_res.c_str(), "", m_statusinfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/dl[2]/dt[4]/text()");
    concatstr(S_res.c_str(), ",", m_statusinfo);
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/dl[2]/dt[5]/text()");
    string str_res = S_res.c_str();
    pos = str_res.find(" - ");
    if(pos != string::npos)
    {
        concatstr(str_res.substr(0, pos).c_str(), ",", m_statusinfo);
        concatstr(str_res.substr(pos + 3).c_str(), ",", m_statusinfo);
    }
    S_res= TinyXPath::S_xpath_string (XEp_main, "//body/dl[2]/dt[8]/text()");
    m_statusinfo += ",\"Server Works\":\"";
    m_statusinfo += S_res.c_str();
    m_statusinfo += "\"";

    delete XDp_doc;

    return 0;
}

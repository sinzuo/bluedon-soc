#include "bd_monitor_base.h"
#include <sstream>


BDMonitorBase::BDMonitorBase()
{
    _interval   = 300; // 5minutes
    _filename   = "";
    _modName    = "";
    _ip         = "";
    _port       = 0;
    _policyItem = 0;

    _errcode    = 0;
    _errdesc    = "";
}

string BDMonitorBase::createHttpHead(const string &cmd, const string &authinfo, const string &ip, int port)
{
    if (cmd.empty() || ip.empty()) return "";

    string head, strport;
    stringstream stream;
    stream << port;
    stream >> strport;

    //组织http请求报文头
    head = string("GET ") + cmd + " HTTP/1.1\r\n";
    head += "Accept: */*\r\nUser-Agent: Mozilla/6.0\r\n";
    head += string("Host: ") + ip + ":" + strport + "\r\n";

    if (!authinfo.empty()) {
        head += "Authorization: Basic ";
        head += authinfo + "\r\n";
    }
    head += "Accept-Language: cn\r\nConnection: Close\r\n\r\n";
    //log_debug("the http head is : %s", head.c_str());
    return head;
}

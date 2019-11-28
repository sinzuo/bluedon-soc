
#ifndef BD_HOSTINFO_H_
#define BD_HOSTINFO_H_

#include "bd_basic.h"
#include <string>
using namespace std;

// 协议类型
enum bd_prottype
{
    PT_OTHER,
    PT_SNMP, PT_SSH, PT_TELNET,
    PT_BDSEC,PT_LOCAL
};

// 主机信息
struct bd_host
{
    bd_longlong hostid;
    bd_prottype prottype;   ///< 管控协议类型

    string ipv4;            ///< 主机IP
    int port;               ///< potocal port
    string username;        ///< 查询（获取状态值）用户
    string password;        ///< 查询（获取状态值）密码
    int snmpversion;
    string community; 
    string bdsec_cert_file;	///< bdsec的证书路径
    
    bd_host() : hostid(0), prottype(PT_OTHER), port(0)
    {
    }
};

#endif // BD_HOSTINFO_H_


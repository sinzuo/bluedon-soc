
#include "bd_topu_task.h"
#include "Poco/Types.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/XML/XMLException.h"
#include <iostream>

using namespace Poco;

CTopu:: CTopu(string pszHost, string pszCommunity, string pszUser, string pszProtol, string pszPassPhrase, long lVersion)
{
    this->m_conn_info.pszHost = pszHost;
    this->m_conn_info.pszCommunity = pszCommunity;
    this->m_conn_info.pszUser = pszUser;
    this->m_conn_info.pszProtol = pszProtol;
    this->m_conn_info.pszPassPhrase = pszPassPhrase;
    this->m_conn_info.lVersion = lVersion;
}

CTopu:: CTopu(string pszHost, string pszCommunity, long lVersion, int secuLevel, string pszUser, string pszProtol, string pszPassPhrase, string privProtol, string privPassPhrase)
{
    this->m_conn_info.pszHost = pszHost;
    this->m_conn_info.pszCommunity = pszCommunity;
    this->m_conn_info.pszUser = pszUser;
    this->m_conn_info.pszProtol = pszProtol;
    this->m_conn_info.pszPassPhrase = pszPassPhrase;
    this->m_conn_info.lVersion = lVersion;
    this->m_conn_info.privProtol = privProtol;
    this->m_conn_info.privPassPhrase = privPassPhrase;
    this->m_conn_info.secuLevel = secuLevel;
}

bool CTopu:: get_info_from_L3(HOSTINFO &hostinfo)
{

    SNMPOID ifrouteroid(".1.3.6.1.2.1.4.1.0");   //用来判断设备是否具有路由转发功能
    SNMPOID bridgeaddressoid(".1.3.6.1.2.1.17.1.1.0");   //设备有没有实现bridge-mib，判断是不是交换机
    SNMPOID hostoid(".1.3.6.1.2.1.25.1.1.0");   //设备有没有实现host-mib，判断是不是主机

    SNMPOID sysdescoid(".1.3.6.1.2.1.1.1.0"); //系统描述 可以用作判断是操作系统
    SNMPOID sysuptimeoid(".1.3.6.1.2.1.1.3.0"); // 系统启动时间
    SNMPOID syscontactoid(".1.3.6.1.2.1.1.4.0"); // 联系方式
    SNMPOID sysnameoid(".1.3.6.1.2.1.1.5.0");   // 系统名字
    SNMPOID syslocationoid(".1.3.6.1.2.1.1.6.0"); // 所在地
    SNMPOID iftableoid(".1.3.6.1.2.1.2.2.1");    //网口信息
    SNMPVALUE sysvalue; //临时变量
    SNMPTABLE valuetable; //临时表变量



    //
    //*需要添加通过ping 判断设备是否连通
    //*用于判断ICMP是否开启
    int timecost = 0;
    //string l3ip =  hostinfo.sysip;
    string l3ip = this->m_conn_info.pszHost;
    string community = this->m_conn_info.pszCommunity;

    bool bConnect = this->getPingState(l3ip.c_str(), 30000, &timecost);
    if (bConnect == 0 )
    {
        hostinfo.ICMP_ENABLE = 0;  //ICMP没有开启
        return true;
    }
    else
    {
        hostinfo.ICMP_ENABLE = 1;  //ICMP开启
    }

    CNetSnmp snmp(this->m_conn_info.pszHost.c_str(), this->m_conn_info.pszCommunity.c_str(), \
                  this->m_conn_info.lVersion, this->m_conn_info.secuLevel ,\
                  this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), \
                  this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.privProtol.c_str(),\
                  this->m_conn_info.privPassPhrase.c_str());

    //pszHost, pszCommunity, lVersion, seculevel, pszUser, pszProtol, pszPassPhrase, p_privProtol, p_privPass
    //CNetSnmp snmp(this->m_conn_info.pszHost.c_str(), this->m_conn_info.pszCommunity.c_str(), this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.lVersion);
    //SNMP_VERSION_3 : pszHost pszUser pszProtol  pszPassPhrase
    //CNetSnmp snmp("172.16.7.1", "public", "snmpv3", "MD5", "bluedonpw", SNMP_VERSION_3);

    //1.是否具有路由转发功能,不能获取则判断为snmp没有开启
    if (snmp.Get_v3(ifrouteroid, &sysvalue) != 0)
    {

        string error = snmp.GetError();
        printf("Failed to get ifrouteroid info from [%s] via SNMP\n",l3ip.c_str());
        printf("error = [%s] via SNMP\n",error.c_str());
        hostinfo.SNMP_ENABLE = 0;  //snmp没有开启
        hostinfo.Dev_Type = 0;   //未知类型
        hostinfo.ifrouter = 0;
        return true;
    }
    else
    {
        hostinfo.SNMP_ENABLE = 1;  //snmp开启
        if (!strcmp(sysvalue.GetValueToString().c_str(),"1"))
        {
            hostinfo.ifrouter = 1;
            hostinfo.Dev_Type = 1;   //有路由转发功能，则为路由器或三层交换机

        }
        else
        {
            hostinfo.ifrouter = 0;
            hostinfo.Dev_Type = 3;   //没有路由转发功能，则为普通交换机或普通主机, 预设为3
        }
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空
    //2.判断设备有没有实现bridge-mib
    if (snmp.Get_v3(bridgeaddressoid, &sysvalue) != 0)
    {
        printf("Failed to get bridgeaddressoid info from [%s] via SNMP\n",l3ip.c_str());
    }
    else
    {
            hostinfo.Dev_Type = 2;  //设备类型为交换机
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空
    //3.判断设备有没有实现host-mib
    if (snmp.Get_v3(hostoid, &sysvalue) != 0)
    {
        printf("Failed to get hostoid info from [%s] via SNMP\n",l3ip.c_str());

        if ( hostinfo.Dev_Type == 3 )
            hostinfo.Dev_Type = 0;  //没有实现host-mib,则为未知设备类型

    }
    else
    {
            hostinfo.Dev_Type = 3;
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空
    //4.获取主机信息
    if (snmp.Get_v3(sysdescoid, &sysvalue) != 0)
    {
        printf("Failed to get sysdescoid info from [%s] via SNMP",l3ip.c_str());
        return true;
    }
    else
    {
        hostinfo.sysdescvalue = sysvalue.GetValueToString();
        //从系统描述中获取操作系统
        if ( hostinfo.sysdescvalue.find("Linux") != string::npos  )
        {
            hostinfo.sysop = 1;
        }
        else if ( hostinfo.sysdescvalue.find("Windows") != string::npos  )
        {
            hostinfo.sysop = 2;
        }
        else if ( hostinfo.sysdescvalue.find("OS X") != string::npos  )
        {
            hostinfo.sysop = 3;
        }
        else
        {
            hostinfo.sysop = 0;
        }
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(sysuptimeoid, &sysvalue) != 0)
    {
        printf("Failed to get sysuptimeoid info from [%s] via SNMP",l3ip.c_str());
        hostinfo.sysuptimevalue = "";
    }
    else
    {
        hostinfo.sysuptimevalue = sysvalue.GetValueToString();
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(syscontactoid, &sysvalue) != 0)
    {
        printf("Failed to get syscontactoid info from [%s] via SNMP",l3ip.c_str());
        hostinfo.syscontactvalue = "";
    }
    else
    {
        hostinfo.syscontactvalue = sysvalue.GetValueToString();
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(sysnameoid, &sysvalue) != 0)
    {
        printf("Failed to get sysnameoid info from [%s] via SNMP",l3ip.c_str());
        hostinfo.sysname = "";
    }
    else
    {
        hostinfo.sysname = sysvalue.GetValueToString();
    }
    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(syslocationoid, &sysvalue) != 0)
    {
        printf("Failed to get syslocationoid info from [%s] via SNMP",l3ip.c_str());
        hostinfo.syslocationvalue = "";
    }
    else
    {
        hostinfo.syslocationvalue = sysvalue.GetValueToString();
    }
    return true;
}

bool CTopu:: get_info_from_host(HOSTINFO &hostinfo, const char *community, const long  lVersion, const int secuLevel, const char *user, const char *protol, const char *passwd,const char *privProtol, const char *privPassPhrase)
{

    SNMPOID ifrouteroid(".1.3.6.1.2.1.4.1.0");   //用来判断设备是否具有路由转发功能
    SNMPOID bridgeaddressoid(".1.3.6.1.2.1.17.1.1.0");   //设备有没有实现bridge-mib，判断是不是交换机
    SNMPOID hostoid(".1.3.6.1.2.1.25.1.1.0");   //设备有没有实现host-mib，判断是不是主机

    SNMPOID sysdescoid(".1.3.6.1.2.1.1.1.0"); //系统描述 可以用作判断是操作系统
    SNMPOID sysuptimeoid(".1.3.6.1.2.1.1.3.0"); // 系统启动时间
    SNMPOID syscontactoid(".1.3.6.1.2.1.1.4.0"); // 联系方式
    SNMPOID sysnameoid(".1.3.6.1.2.1.1.5.0");   // 系统名字
    SNMPOID syslocationoid(".1.3.6.1.2.1.1.6.0"); // 所在地
    SNMPOID iftableoid(".1.3.6.1.2.1.2.2.1");    //网口信息
    SNMPOID ipAddrtableoid(".1.3.6.1.2.1.4.20.1");    //ip地址
    SNMPVALUE sysvalue; //临时变量
    SNMPTABLE valuetable; //临时表变量
    SNMPTABLE valuetable2; //临时表变量

    //
    //*需要添加通过ping 判断设备是否连通
    //*用于判断ICMP是否开启
    int timecost = 0;
    bool bConnect = this->getPingState(hostinfo.sysip.c_str(), 30000, &timecost);
    if (bConnect == 0 )
    {
        hostinfo.ICMP_ENABLE = 0;  //ICMP没有开启
        return true;
    }
    else
    {
        hostinfo.ICMP_ENABLE = 1;  //ICMP开启
    }


    //CNetSnmp snmp(hostinfo.sysip.c_str(), community, user, protol, passwd, lVersion);
    CNetSnmp snmp(hostinfo.sysip.c_str(), community, lVersion, secuLevel, user, protol, passwd, privProtol, privPassPhrase);

    /*CNetSnmp snmp(hostinfo.sysip.c_str(), this->m_conn_info.pszCommunity.c_str(), \
                  this->m_conn_info.lVersion, this->m_conn_info.secuLevel ,\
                  this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), \
                  this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.privProtol.c_str(),\
                  this->m_conn_info.privPassPhrase.c_str());
    */
    //1.是否具有路由转发功能,不能获取则判断为snmp没有开启
    if (snmp.Get_v3(ifrouteroid, &sysvalue) != 0)
    {
        printf("Failed to get ifrouteroid info from [%s] via SNMP\n",hostinfo.sysip.c_str());
        hostinfo.SNMP_ENABLE = 0;  //snmp没有开启
        hostinfo.Dev_Type = 0;   //未知类型
        hostinfo.ifrouter = 0;
        return true;
    }
    else
    {
        hostinfo.SNMP_ENABLE = 1;  //snmp开启
        if (!strcmp(sysvalue.GetValueToString().c_str(),"1"))
        {
            hostinfo.ifrouter = 1;
            hostinfo.Dev_Type = 1;   //有路由转发功能，则为路由器或三层交换机

        }
        else
        {
            hostinfo.ifrouter = 0;
            hostinfo.Dev_Type = 3;   //没有路由转发功能，则为普通交换机或普通主机, 预设为3
        }
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空
    //2.判断设备有没有实现bridge-mib
    if (snmp.Get_v3(bridgeaddressoid, &sysvalue) != 0)
    {
        printf("Failed to get bridgeaddressoid info from [%s] via SNMP\n",hostinfo.sysip.c_str());
    }
    else
    {
            hostinfo.Dev_Type = 2;  //设备类型为交换机
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空
    //3.判断设备有没有实现host-mib
    if (snmp.Get_v3(hostoid, &sysvalue) != 0)
    {
        printf("Failed to get hostoid info from [%s] via SNMP\n",hostinfo.sysip.c_str());

        if ( hostinfo.Dev_Type == 3 )
            hostinfo.Dev_Type = 0;  //没有实现host-mib,则为未知设备类型

    }
    else
    {
            hostinfo.Dev_Type = 3;
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空
    //4.获取主机信息
    if (snmp.Get_v3(sysdescoid, &sysvalue) != 0)
    {
        printf("Failed to get sysdescoid info from [%s] via SNMP",hostinfo.sysip.c_str());
        return true;
    }
    else
    {
        hostinfo.sysdescvalue = sysvalue.GetValueToString();
        //从系统描述中获取操作系统
        if ( hostinfo.sysdescvalue.find("Linux") != string::npos  )
        {
            hostinfo.sysop = 1;
        }
        else if ( hostinfo.sysdescvalue.find("Windows") != string::npos  )
        {
            hostinfo.sysop = 2;
        }
        else if ( hostinfo.sysdescvalue.find("OS X") != string::npos  )
        {
            hostinfo.sysop = 3;
        }
        else
        {
            hostinfo.sysop = 0;
        }
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    //根据操作系统，设备种类来设置设备类型
    switch(hostinfo.Dev_Type)
    {
        case 0:
            hostinfo.NEW_Dev_Type = 40000;
        break;
        case 1:
            hostinfo.NEW_Dev_Type = 11000;
           break;
        case 2:
            hostinfo.NEW_Dev_Type = 12000;
           break;
        case 3:
            if (hostinfo.sysop == 1)
                hostinfo.NEW_Dev_Type = 22000;
            else if (hostinfo.sysop == 2)
                hostinfo.NEW_Dev_Type = 21000;
            else
                hostinfo.NEW_Dev_Type = 20000;
         break;

        default :
        hostinfo.NEW_Dev_Type = 40000;
        break;
    }

    if (snmp.Get_v3(sysuptimeoid, &sysvalue) != 0)
    {
        printf("Failed to get sysuptimeoid info from [%s] via SNMP",hostinfo.sysip.c_str());
        hostinfo.sysuptimevalue = "";
    }
    else
    {
        hostinfo.sysuptimevalue = sysvalue.GetValueToString();
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(syscontactoid, &sysvalue) != 0)
    {
        printf("Failed to get syscontactoid info from [%s] via SNMP",hostinfo.sysip.c_str());
        hostinfo.syscontactvalue = "";
    }
    else
    {
        hostinfo.syscontactvalue = sysvalue.GetValueToString();
    }

    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(sysnameoid, &sysvalue) != 0)
    {
        printf("Failed to get sysnameoid info from [%s] via SNMP",hostinfo.sysip.c_str());
        hostinfo.sysname = "";
    }
    else
    {
        hostinfo.sysname = sysvalue.GetValueToString();
    }
    memset(&sysvalue, 0, sizeof(sysvalue)); //清空

    if (snmp.Get_v3(syslocationoid, &sysvalue) != 0)
    {
        printf("Failed to get syslocationoid info from [%s] via SNMP",hostinfo.sysip.c_str());
        hostinfo.syslocationvalue = "";
    }
    else
    {
        hostinfo.syslocationvalue = sysvalue.GetValueToString();
    }

    //获取ip地址
    if (snmp.GetTable_v3(ipAddrtableoid, &valuetable) != 0)
    {
        printf("Failed to get ipAddrtableoid info from [%s] via SNMP", hostinfo.sysip.c_str());
        return false;
    }
    for(int i = 1; i <= valuetable.count; i++ )
    {
        SNMPTABLEROW *row = valuetable[i];
        LPSNMPTABLECOLUMN ipAddrEnd;

        if ((*row)[1])
            ipAddrEnd = (*row)[1];
        else
            continue;

        string ipaddr = ipAddrEnd->column_value.GetValueToString();

        if (ipaddr.empty() || ipaddr == "127.0.0.1") {
            continue;
        }
        hostinfo.Dev_IP.insert(ipaddr);
        row->Free();
    }
    valuetable.Free();
    hostinfo.IP_Count = hostinfo.Dev_IP.size();
    //获取MAC地址
    if (snmp.GetTable_v3(iftableoid, &valuetable2) != 0)
    {
        printf("Failed to get iftableoid info from [%s] via SNMP", hostinfo.sysip.c_str());
        return false;
    }
    for(int i = 1; i <= valuetable2.count; i++ )
    {
        SNMPTABLEROW *row = valuetable2[i];
        LPSNMPTABLECOLUMN physAddr;
        LPSNMPTABLECOLUMN ifstat;

        if ((*row)[6])
            physAddr = (*row)[6];
        else
            continue;
        if ((*row)[8])
            ifstat = (*row)[8];
        else
            continue;

       //string macaddr = _fill_mac_with_zero(physAddr->column_value.GetValueToString());
        string macaddr = physAddr->column_value.GetValueToString();
        string::size_type pos = 0;
        int number = 0;
        while(pos != string::npos)
        {
            pos = macaddr.find(":", pos);
            if(pos != string::npos)
            {
                ++number;
                pos += (string(":").size() + 1);
            }
        }
        if (number > 4)
        {
             macaddr = _fill_mac_with_zero(macaddr);
        }
        else
        {
            macaddr = this->StringToHexString(macaddr);
        }

        int ifstatoct = ifstat->column_value.nInterValue;//网口是否开启 1开启 2关闭
        if (macaddr.empty()|| macaddr == "0" || macaddr == "20" ||ifstatoct == 2) {
            continue;
        }
        hostinfo.Dev_MAC.insert(macaddr);
        row->Free();
    }
    valuetable2.Free();
    hostinfo.MAC_Count = hostinfo.Dev_MAC.size();
    return true;
}

bool CTopu:: get_info_from_ipRouteTable(const string& routerip, map<string, bd_ulonglong>* host)
{

    if (routerip.empty() || NULL == host)
    {
        printf("ip is empty or hostlist is NULL");
        return false;
    }

    CNetSnmp snmp(this->m_conn_info.pszHost.c_str(), this->m_conn_info.pszCommunity.c_str(), \
                  this->m_conn_info.lVersion, this->m_conn_info.secuLevel ,\
                  this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), \
                  this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.privProtol.c_str(),\
                  this->m_conn_info.privPassPhrase.c_str());

    //CNetSnmp snmp(routerip.c_str(), "public", "", "", "", NETSNMP_VERSION_2c);
    //CNetSnmp snmp(routerip.c_str(), this->m_conn_info.pszCommunity.c_str(), this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.lVersion);
    SNMPOID ipRouteTable(".1.3.6.1.2.1.4.21.1");
    SNMPTABLE valuetable;
    if (snmp.GetTable_v3(ipRouteTable, &valuetable) != 0)
    {
        printf("Failed to get host info from [%s] via SNMP", routerip.c_str());
        return false;
    }
    //printf("count [%d]", valuetable.count);
    for(int i = 1; i <= valuetable.count; i++ )
    {
        SNMPTABLEROW *row = valuetable[i];
        LPSNMPTABLECOLUMN node;
        LPSNMPTABLECOLUMN netaddr;
        LPSNMPTABLECOLUMN ipRouteType;

        if ((*row)[1])
            netaddr = (*row)[1];
        else
            continue;

        if ((*row)[2])
            node = (*row)[2];
        else
            continue;

        if ((*row)[8])
            ipRouteType = (*row)[8];
        else
            continue;

        long long RouteType = ipRouteType->column_value.nInterValue;
        long long nodeid = node->column_value.nInterValue;
        string ipaddr = netaddr->column_value.GetValueToString();

        //RouteType == 3 的为直连子网
        if (ipaddr.empty() || RouteType != 3) {
            continue;
        }
        host->insert(make_pair(ipaddr, nodeid));
        row->Free();
    }
    valuetable.Free();
    return true;
}

bool CTopu:: get_L3_info_from_ipRouteTable(list<L3_TOPO_INFO> &l3_info_list, set<string> &L3routelist)
{

    list<string> routelist;//待查找的路由
    list<string> search_routelist;//记录已经寻找过的路由

    string l3_ip = this->m_conn_info.pszHost;
    string community = this->m_conn_info.pszCommunity;
    routelist.push_back(l3_ip);
    search_routelist.push_back(l3_ip);

    SNMPOID ipRouteTable(".1.3.6.1.2.1.4.21.1");

    while (!routelist.empty())
    {
        SNMPTABLE valuetable;
        string CurrentRouterip = routelist.front();
        routelist.pop_front();
        CNetSnmp snmp(CurrentRouterip.c_str(), this->m_conn_info.pszCommunity.c_str(), \
                      this->m_conn_info.lVersion, this->m_conn_info.secuLevel ,\
                      this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), \
                      this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.privProtol.c_str(),\
                      this->m_conn_info.privPassPhrase.c_str());
        //CNetSnmp snmp(CurrentRouterip.c_str(), this->m_conn_info.pszCommunity.c_str(), this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.lVersion);
        //CNetSnmp snmp(CurrentRouterip.c_str(), community.c_str(), "", "", "", NETSNMP_VERSION_2c);
        if (snmp.GetTable_v3(ipRouteTable, &valuetable) != 0)
        {
            printf("Failed to get RouteTable info from [%s] via SNMP\n", CurrentRouterip.c_str());
            valuetable.Free();
            continue;
        }

        L3routelist.insert(CurrentRouterip); //记录能访问的路由，用于获取子网设备
        for(int i = 1; i <= valuetable.count; i++ )
        {
            SNMPTABLEROW *row = valuetable[i];
            LPSNMPTABLECOLUMN node;
            LPSNMPTABLECOLUMN netaddr;
            LPSNMPTABLECOLUMN ipRouteType;
            LPSNMPTABLECOLUMN netMask;
            LPSNMPTABLECOLUMN nextHop;

            if ((*row)[1])
                netaddr = (*row)[1];
            else
                continue;
            if ((*row)[2])
                node = (*row)[2];
            else
                continue;
            if ((*row)[7])
                nextHop = (*row)[7];
            else
                continue;
            if ((*row)[8])
                ipRouteType = (*row)[8];
            else
                continue;
            if ((*row)[11])
                netMask = (*row)[11];
            else
                continue;

            long long nodeid = node->column_value.nInterValue;
            long long RouteType = ipRouteType->column_value.nInterValue;
            string ipaddr = netaddr->column_value.GetValueToString();
            string ipmask = netMask->column_value.GetValueToString();
            string ipnexthop = nextHop->column_value.GetValueToString();
            L3_TOPO_INFO l3_info_temp;

            l3_info_temp.Dst_ID = 0;
            l3_info_temp.Src_ID = 0;
			
			l3_info_temp.Dstissubnet = false; //判断目的ip是否子网ip
            if (ipnexthop == "127.0.0.1") //忽略lo回路
                goto END;
			
            //RouteType == 3 的为直连子网
            if (RouteType == 3)
            {               
                if(ipmask == "255.255.255.255")
                {
                    list<string>::iterator stringIter = find(search_routelist.begin(), search_routelist.end(), ipnexthop);
                    if(stringIter != search_routelist.end())
                            continue;
                    routelist.push_back(ipnexthop);
                    search_routelist.push_back(ipnexthop);
                    l3_info_temp.Src_IP = CurrentRouterip;
                    l3_info_temp.Dst_IP = ipnexthop;
                    l3_info_temp.Node = nodeid;
                    //l3_info_temp.Mask = ipmask;

                }
                else
                {
                    l3_info_temp.Src_IP = CurrentRouterip;
                    //l3_info_temp.Dst_IP = ipaddr;
                    l3_info_temp.Dst_IP = ipaddr;
                    l3_info_temp.Node = nodeid;
                    l3_info_temp.Mask = ipmask;
                    l3_info_temp.Dstissubnet = true;

                }
            }
            /* else if (RouteType == 4)
            {
                if ( ipmask == "0.0.0.0" )
                    continue;
                list<string>::iterator stringIter = find(search_routelist.begin(), search_routelist.end(), ipnexthop);
                if(stringIter != search_routelist.end())
                    continue;
                routelist.push_back(ipnexthop);
                search_routelist.push_back(ipnexthop);
                l3_info_temp.Src_IP = CurrentRouterip;
                l3_info_temp.Dst_IP = ipnexthop;
                l3_info_temp.Node = nodeid;
                //l3_info_temp.Mask = ipmask;
            } */
            else
                goto END;

            l3_info_list.push_back(l3_info_temp);
        END:
            row->Free();
        }
        valuetable.Free();

    }

    return true;
}


bool CTopu::getPingState(const string &strIP, int ntimeout, int *timecost) {
    bd_ping bdPing;
    bool bConnect = bdPing.ping(strIP, ntimeout, timecost);
    return bConnect;
}


bool CTopu::get_L2info_from_ipNetToMediaTable(const set<string>& routerip_set, list<L3_TOPO_INFO> &l3_topuinfo_list, vector<L2_TOPO_INFO> &l2_topuinfo_list, map<string,string> &all_info_ip)
{
   // printf(bd_netdetect_topology, _get_port_to_host_from_ipNetToMediaTable);

    all_info_ip.clear(); //存放子网所有ip
    string community = this->m_conn_info.pszCommunity;
    SNMPTABLE valuetable;

    if (routerip_set.empty())
    {
        printf("ip is empty or hostlist is NULL");
        return false;
    }
    for (list<L3_TOPO_INFO>::iterator it = l3_topuinfo_list.begin(); it != l3_topuinfo_list.end(); ++it)
    {
        L2_TOPO_INFO l2_topuinfo;
        l2_topuinfo.Parent_Node_ID  = it->Src_ID;
        l2_topuinfo.Parent_Node_IP  = it->Src_IP;
        l2_topuinfo.Sub_Node_ID  = it->Dst_ID;
        l2_topuinfo.Sub_Node_IP= it->Dst_IP;
        l2_topuinfo.Parent_Node_IF_Index= it->Node;
        l2_topuinfo_list.push_back(l2_topuinfo);

        if(it->Dstissubnet)
        {
            all_info_ip.insert(make_pair(it->Dst_IP, string("subnet")));  //如果设备是子网，把mac地址置为subnet
        }
        else
        {
            all_info_ip.insert(make_pair(it->Dst_IP, string("")));
        }

    }

   //需要加上循环routeip

    for (set<string>::iterator it = routerip_set.begin(); it != routerip_set.end(); it++)
    {
        all_info_ip.insert(make_pair(*it, string("")));


        CNetSnmp snmp((*it).c_str(), this->m_conn_info.pszCommunity.c_str(), \
                      this->m_conn_info.lVersion, this->m_conn_info.secuLevel ,\
                      this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), \
                      this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.privProtol.c_str(),\
                      this->m_conn_info.privPassPhrase.c_str());

        //CNetSnmp snmp((*it).c_str(), this->m_conn_info.pszCommunity.c_str(), this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.lVersion);
        //CNetSnmp snmp((*it).c_str(), community.c_str(), "", "", "", NETSNMP_VERSION_2c);
        SNMPOID ipNetToMediaTable(".1.3.6.1.2.1.4.22.1");

        if (snmp.GetTable_v3(ipNetToMediaTable, &valuetable) != 0)
        {
            printf("Failed to get host info from [%s] via SNMP", (*it).c_str());
            return false;
        }

        for(int i = 1; i <= valuetable.count; i++ )
        {
            SNMPTABLEROW *row = valuetable[i];
            LPSNMPTABLECOLUMN physport;
            LPSNMPTABLECOLUMN netaddr;
            LPSNMPTABLECOLUMN netmac;

            if ((*row)[1])
               physport = (*row)[1];
            else
                continue;
            if ((*row)[2])
                netmac = (*row)[2];
            else
                continue;
            if ((*row)[3])
                netaddr = (*row)[3];
            else
                continue;
            //printf("port:%llu\n",physport->column_value.nInterValue);
            bd_ulonglong port = physport->column_value.nInterValue;
            string ip = netaddr->column_value.GetValueToString();

            //string mac = _fill_mac_with_zero(netmac->column_value.GetValueToString());
            string mac = netmac->column_value.GetValueToString();
            string::size_type pos = 0;
            int number = 0;
            while(pos != string::npos)
            {
                pos = mac.find(":", pos);
                if(pos != string::npos)
                {
                    ++number;
                    pos += (string(":").size() + 1);
                }
            }
            if (number > 4)
            {

                 mac = _fill_mac_with_zero(mac);
            }
            else
            {
                mac = this->StringToHexString(mac);
            }


            //physport == 0 的为不活动记录
           // if (ip.empty() || port == 0) {
            if (ip.empty()) {
                continue;
            }

            all_info_ip.insert(make_pair(ip,mac));

            L2_TOPO_INFO l2_topuinfo;

            l2_topuinfo.Sub_Node_Name = "";
            l2_topuinfo.Sub_Node_IP = ip;
            l2_topuinfo.Sub_Node_IF_Name = "";
            l2_topuinfo.Sub_Node_IF_Index = 0;
            l2_topuinfo.Sub_Node_IF_MAC = mac;
            l2_topuinfo.Sub_Node_Dev_Type = 0;
            l2_topuinfo.Sub_Node_ID = 0;
            l2_topuinfo.Parent_Node_IF_Index = port;
            int ip_data[4];
            int net_data[4];
            int mask_data[4];
            sscanf(ip.c_str(), "%d.%d.%d.%d", &ip_data[0], &ip_data[1], &ip_data[2], &ip_data[3]);

            for (list<L3_TOPO_INFO>::iterator it = l3_topuinfo_list.begin(); it != l3_topuinfo_list.end(); ++it)
            {
                if(it->Mask.empty())
                    continue;
                sscanf(it->Dst_IP.c_str(), "%d.%d.%d.%d", &net_data[0], &net_data[1], &net_data[2], &net_data[3]);
                sscanf(it->Mask.c_str(), "%d.%d.%d.%d", &mask_data[0], &mask_data[1], &mask_data[2], &mask_data[3]);

                if((ip_data[0]&mask_data[0]) == (net_data[0]&mask_data[0]) &&
                        (ip_data[1]&mask_data[1]) == (net_data[1]&mask_data[1]) &&
                        (ip_data[2]&mask_data[2]) == (net_data[2]&mask_data[2]) &&
                        (ip_data[3]&mask_data[3]) == (net_data[3]&mask_data[3]))
                {

                    l2_topuinfo.Parent_Node_ID = it->Dst_ID;
                    l2_topuinfo.Parent_Node_IP = it->Dst_IP;
                    l2_topuinfo.Parent_Node_Name = ""; //需要补充
                    l2_topuinfo.Parent_Node_IF_Name = ""; //需要补充
                    l2_topuinfo.Parent_Node_IF_MAC = "";//需要补充
                    l2_topuinfo.Parent_Node_Dev_Type = 0;//需要补充
                }


                //if (it->Node == l2_topuinfo.Parent_Node_IF_Index)
                //{
                //    l2_topuinfo.Parent_Node_ID = it->Src_ID;
                //    l2_topuinfo.Parent_Node_IP = it->Dst_IP;
                //}

            }
            l2_topuinfo_list.push_back(l2_topuinfo);

            row->Free();
        }

    }
    valuetable.Free();
    return true;
}

bool CTopu:: get_mac_ifdesc_byip(const string routerip, const int ifnum, string &ifdesc, string &mac)
{

    if (routerip.empty() )
    {
        printf("ip is empty or hostlist is NULL");
        return false;
    }
    string community = this->m_conn_info.pszCommunity;
    CNetSnmp snmp(routerip.c_str(), this->m_conn_info.pszCommunity.c_str(), \
                  this->m_conn_info.lVersion, this->m_conn_info.secuLevel ,\
                  this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), \
                  this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.privProtol.c_str(),\
                  this->m_conn_info.privPassPhrase.c_str());
    //CNetSnmp snmp(routerip.c_str(), community.c_str(), "", "", "", NETSNMP_VERSION_2c);
    //CNetSnmp snmp(routerip.c_str(), community.c_str(), this->m_conn_info.pszUser.c_str(), this->m_conn_info.pszProtol.c_str(), this->m_conn_info.pszPassPhrase.c_str(), this->m_conn_info.lVersion);
    SNMPOID iftableoid(".1.3.6.1.2.1.2.2.1");
    SNMPTABLE valuetable;
    if (snmp.GetTable_v3(iftableoid, &valuetable) != 0)
    {
        printf("Failed to get iftable info from [%s] via SNMP", routerip.c_str());
        return false;
    }
    //printf("count [%d]", valuetable.count);
    for(int i = 1; i <= valuetable.count; i++ )
    {
        SNMPTABLEROW *row = valuetable[i];
        LPSNMPTABLECOLUMN index;
        LPSNMPTABLECOLUMN descr;
        LPSNMPTABLECOLUMN macaddr;

        if ((*row)[1])
            index = (*row)[1];
        else
            continue;

        if ((*row)[2])
            descr = (*row)[2];
        else
            continue;

        if ((*row)[6])
            macaddr = (*row)[6];
        else
            continue;

        int cur_index = index->column_value.nInterValue;
        string cur_descr = descr->column_value.GetValueToString();;
        string cur_macaddr = _fill_mac_with_zero(macaddr->column_value.GetValueToString());

        //获取端口对应的名称跟mac
        if (cur_index == ifnum)
        {
            ifdesc = cur_descr;
            mac = cur_macaddr;
            return true;
        }
    }
    ifdesc = "";
    mac = "";
    valuetable.Free();
    return false;
}


string CTopu:: _fill_mac_with_zero(const string& src)
{
    //BD_TRACE(bd_netdetect_topology, _fill_mac_with_zero);
    if (src.empty())
    {
       // BD_LOGERR("src is empty");
        return 0;
    }

    string dst;
    string zero = "0";
    vector<string> macvec;
    bd_split(src, ":", &macvec);
    for (vector<string>::iterator it = macvec.begin(); it != macvec.end(); ++it)
    {
        if (! dst.empty())
        {
            dst.append(":");
        }
        if (it->size() != 2)
        {
            string temp = zero + *it;
            dst.append(temp);
        }
        else
        {
            dst.append(*it);
        }
    }
    return dst;
}

void CTopu::bd_split(const std::string& buff,
              const std::string& sep,
              std::vector<std::string>* listSplit)
{
    int start = 0;
    int end = 0;

    do {
        end = buff.find(sep, start);
        if(end < 0) {
            listSplit->push_back(buff.substr(start));
            return;
        } else {
            listSplit->push_back(buff.substr(start, end - start));
            start = end + sep.size();
        }

    } while(1);
}



string CTopu::StringToHexString(std::string &strHexSrc)  //把乱码的mac地址转换
{
    string temp;
    int nlen = strHexSrc.length();
    //if (nlen != 6)
     //   return temp;
    char chTmp[512] = { 0 };

    for (int i = 0; i<nlen ; i++)
    {
        sprintf(chTmp,"%02x:",strHexSrc[i]&0xff);
        //sprintf(chTmp,"%02x:",strHexSrc[i]);
        temp += string(chTmp);

    }
    temp.erase(temp.end()-1); //删除最后一个：
    return temp;
}

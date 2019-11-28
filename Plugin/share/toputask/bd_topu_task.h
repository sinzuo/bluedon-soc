#ifndef _BD_TASK_H_
#define _BD_TASK_H_

#include "icmp/bd_ping.h"
#include "https/bd_https_control.h"
#include "snmp/bd_snmp.h"
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

#define PROTOCOL_BDSEC "BDSEC"
#define PROTOCOL_SNMP  "SNMP" 

#define BDSECXML_NODENAME_CPU     "CPU"
#define BDSECXML_NODENAME_MEM     "GetMemory"
#define BDSECXML_NODENAME_DISK    "Harddisk"
#define BDSECXML_NODENAME_SERVICE "Service"

//Poco的xm模型是从第二级节点开始
#define BDSECXML_NODEPATH_CUP     "/Action/GetCPUs" 
#define BDSECXML_NODEPATH_MEM     "/Action" 
#define BDSECXML_NODEPATH_DISK    "/Action/GetHarddisks" 
#define BDSECXML_NODEPATH_SERVICE "/Action/Getservices" 

#define GET_XML_ATTR(node,name,save)\
	if( node->hasAttribute(#name))\
        save = node->getAttribute(#name);


typedef struct _HOSTINFO
{
    //网元id
    int recordid;
    //设备ip地址
    string sysip;
    //设备mac地址
    string devmac;
    //系统描述
    string sysdescvalue;
    //操作系统  0:未知; 1:linux; 2：windows; 3 osx;
    string Model_Number;
    //厂家型号
    int sysop;
    //开启时间
    string sysuptimevalue;
    //联系方式
    string syscontactvalue;
    //系统名称
    string sysname;
    //系统所在地
    string syslocationvalue;
    //节点设备类型	0:未知; 1:交换机; 2：路由器; 3普通主机;
    int  Dev_Type;
    //是否支持SNMP    0：否；1：是
    int  SNMP_ENABLE;
    //是否支持ICMP    0：否；1：是
    int  ICMP_ENABLE;
    //是否具有转发功能 0：否；1：是
    int  ifrouter;

    int  IP_Count;
    set<string> Dev_IP; //IP组
    int  MAC_Count;
    set<string> Dev_MAC; //MAC组

    int NEW_Dev_Type; //适配win版topu设备类型

    //接口流量信息
    //set<IFINFO> ifinfo_set;


    _HOSTINFO() //初始化
    {
        recordid = 0;
        Dev_Type = 0;
        SNMP_ENABLE = 0;
        ICMP_ENABLE = 0;
        ifrouter = 0;
        sysop = 0;
        MAC_Count = 0;
        IP_Count = 0;
        NEW_Dev_Type = 40000; //位置设备
        sysip = "";
        sysname = "";
        devmac = "";
        sysdescvalue = "";
        Model_Number = "";
        sysuptimevalue = "";
        syscontactvalue = "";
        syslocationvalue = "";

        Dev_IP.clear();
        Dev_MAC.clear();
    }
}HOSTINFO, *LPHOSTINFO;


/*网络层拓扑信息*/
typedef struct _L3_TOPO_INFO_
{
    int        Src_ID;                 //路由器源ID
    string     Src_IP;     //路由器源IP
    int        Dst_ID;                 //路由器目的ID
    string     Dst_IP;     //路由器目的IP
    long long  Node;                   //端口
    string     Mask;       //mask
    bool Dstissubnet;

}L3_TOPO_INFO, *pL3TopoInfo;

/*拓扑信息*/
typedef struct _L2_TOPO_INFO
{
    int     Sub_Node_ID;				//子节点ID
    string  Sub_Node_Name;            //子节点名称
    string  Sub_Node_IP;                //子节点IP
    string  Sub_Node_IF_Name;         //子节点的端口名称
    int     Sub_Node_IF_Index;			//子节点的端口索引号
    string  Sub_Node_IF_MAC;        //子节点的端口MAC地址
    int     Sub_Node_Dev_Type;		//子节点设备类型
    int     Parent_Node_ID;			//父节点ID
    string  Parent_Node_Name;         //父节点名称
    string  Parent_Node_IP;			 //父节点IP
    string  Parent_Node_IF_Name;    //父节点的端口名称
    int     Parent_Node_IF_Index;	//父节点的端口索引号
    string  Parent_Node_IF_MAC;     //父节点的端口MAC地址
    int     Parent_Node_Dev_Type;   //父节点设备类型

    _L2_TOPO_INFO() //初始化
    {
        Sub_Node_ID = 0;
        Sub_Node_IF_Index = 0;
        Sub_Node_Dev_Type = 0;
        Parent_Node_ID = 0;
        Parent_Node_IF_Index = 0;
        Parent_Node_Dev_Type = 0;
    }

}L2_TOPO_INFO, *pL2TopoInfo;

/*snmp 连接信息*/
typedef struct _SNMP_CONN_INFO
{
    string pszHost;
    string pszCommunity;
    string pszUser;
    string pszProtol;
    string pszPassPhrase;
    string privProtol;
    string privPassPhrase;
    int  secuLevel;          //安全等级
    int  freq;                 //频率
    long lVersion;

}SNMP_CONN_INFO;

class CTopu {

public:
    //利用参数初始化snmp
    CTopu();
    CTopu(string pszHost, string pszCommunity, string pszUser, string pszProtol, string pszPassPhrase, long lVersion);
    CTopu(string pszHost, string pszCommunity, long lVersion, int secuLevel, string pszUser, string pszProtol, string pszPassPhrase, string privProtol, string privPassPhrase);

    ~CTopu(){}
public:
	bool getPingState(const string &strIP, int ntimeout, int *timecost);
    string _fill_mac_with_zero(const string& src);
    void bd_split(const std::string& buff, const std::string& sep, vector<std::string>* listSplit);
    string StringToHexString(std::string &strHexSrc);
    //topu

    bool get_info_from_L3(HOSTINFO &hostinfo);
    //bool get_info_from_host(HOSTINFO &hostinfo, const char *community, const char *user, const char *protol, const char *passwd, const long  lVersion);
    bool get_info_from_host(HOSTINFO &hostinfo, const char *community, const long  lVersion, const int secuLevel, const char *user, const char *protol, const char *passwd,const char *privProtol, const char *privPassPhrase);
    bool get_info_from_ipRouteTable(const string& routerip, map<string, bd_ulonglong>* host);
    bool get_L3_info_from_ipRouteTable(list<L3_TOPO_INFO> &l3_info_list, set<string> &L3routelist);
    bool get_L2info_from_ipNetToMediaTable(const set<string>& routerip, list<L3_TOPO_INFO> &l3_topuinfo_list, vector<L2_TOPO_INFO> &l2_topuinfo_list, map<string,string> &all_info_ip);
    bool get_mac_ifdesc_byip(const string routerip, const int ifnum, string &ifdesc, string &mac);
private:
    SNMP_CONN_INFO m_conn_info;
	vector<string> m_vecProtoParam;  
    vector<HOSTINFO> m_vecHostInfo;

   // static int dev_id;  //设备递增id
    //CNetSnmp snmp;
};

#endif //_BD_TASK_H_





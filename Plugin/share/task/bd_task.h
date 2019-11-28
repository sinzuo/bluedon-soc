#ifndef _BD_TASK_H_
#define _BD_TASK_H_

#include "icmp/bd_ping.h"
#include "https/bd_https_control.h"
#include "snmp/bd_snmp_interface.h"
#include "docker/bd_docker.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

#define PROTOCOL_BDSEC "BDSEC"
#define PROTOCOL_SNMP  "SNMP" 
#define PROTOCOL_DOCKER "DOCKER"

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


class CTask {
	
public:
	CTask(vector<string> &vec):m_vecProtoParam(vec){}
	~CTask(){} 
public:
	bool getPingState(const string &strIP, int ntimeout, int *timecost);
    bool getSysInfo(string &sysinfo);
	bool getCpuUsage( int &nCpu);
	bool getMemUsage( int &nMem);
	bool getDiskUsage( map<string,string> &mapDiskUsage);
 	bool getServiceState( map<string,string> &mapServiceState);
    bool getNetworkUsage(NetWorkInfoList &lis);
    bool getSWInstalledUsage(SWInstallInfoList &lis);
    bool getVaFromXml(const string &strXml, const char *chnodePath, const char *chnodeName, map<string, string> &mapResult);
    //管控指令
    string commond_reboot();
    string commond_shutdown(); 
    string commond_factory_recover();
    string commond_restart_service();
    string commond_stop_service();
    string commond_syntime();
    
    /*本期版本须讨论后再开发相应功能
    void backuppolicy(const string& filetype,const string& savefilename);
    string uploadpolicy(const string& filetype,const string& filename);
    string ipsession(const string& target,const string& action,
			const string& enable,const string& protocol,const string& srcip,
			const string& srcport,const string& desip,const string& desport);
    string macmanage(const struct mac_policy& macpolicy);
    string scannet(const string& beginip,const string& endip);
    void flowmonitor(const string& filetype,const string& savefilename);
    */
private:
	vector<string> m_vecProtoParam;  
};

#endif //_BD_TASK_H_





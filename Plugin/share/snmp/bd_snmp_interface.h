#ifndef __BD_SNMP_INTERFACE__
#define __BD_SNMP_INTERFACE__

#include <vector>
#include "bd_snmp.h"

#ifndef __cplusplus
extern "C"{
#endif

	typedef struct tagNetwork{
		//网口名称，MAC地址，网口状态，流入流量，流出流量
		char if_name[512];
		char if_mac[128];
		int  if_status; //1:up  2:down
		long long  if_inOctet;
		long long  if_outOctet;
	}NetworkInfo,*pNetworkInfo;
	
	typedef std::vector<NetworkInfo>  NetWorkInfoList;
	

	typedef struct tagSWInstall{
		//软件名称，软件类型，安装时间
		char sw_name[512];
		int sw_type; //1:unknown  2:operatingSystem 3:deviceDriver 4:application
		char sw_date[128];
	}SWInstallInfo,*pSWInstallInfo;
	
	typedef std::vector<SWInstallInfo>  SWInstallInfoList;

	typedef struct tagProcess{
		//进程名称，程序路径，进程状态
		char process_name[512];
		char process_path[512];
		int  process_status; //1:running  2:runnable 3:notRunnable 4:invalid
	}ProcessInfo,*pProcessInfo;
	
	typedef std::vector<ProcessInfo>  ProcessInfoList;
	
	typedef struct tagPartitionInfo{
		//类型，分区名字，使用率
		char partition_type[128];
		char partition_name[512];
		double  partition_usage;
        int disk_total;
        int disk_used;
	}PartitionInfo,*pPartitionInfo;
	
	typedef std::vector<PartitionInfo>  DiskInfoList;

	typedef struct tagDeviceInfo{
        //类型，设备名字
		char device_type[128];
		char device_name[512];
	}DeviceInfo,*pDeviceInfo;
	typedef std::vector<DeviceInfo> DeviceList;


	class CSnmpV1{
	public:
		//入口参数有设备IP，snmp团体名，snmp端口（snmp版本和OID可以自己写死）
        //改成v3版本 2017.03.24
        CSnmpV1(const char *pszHost, const char *pszCommunity, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const long lVersion );
        CSnmpV1(const char *pszHost, const char *pszCommunity, const long lVersion, const int seculevel, const char *pszUser, const char *pszProtol, const char *pszPassPhrase, const char *privProtol, const char *privPass);

        ~CSnmpV1();
        int SNMP_GetSysInfo(std::string &sysinfo);
		int SNMP_GetNetworkList(NetWorkInfoList &lis);
		int SNMP_GetSWInstalledList(SWInstallInfoList &lis);
		int SNMP_GetSWRunStatusList(ProcessInfoList &lis);
		int SNMP_GetDiskUsageList(DiskInfoList &lis);
		int SNMP_GetDeviceList(DeviceList &lis);
		int SNMP_GetCpuUsage(double *value);
		int SNMP_GetRamUsage(double *value);
        int SNMP_GetSwitchType(SNMPVALUE sysvalue);
        int SNMP_GetTotalOfListValue(SNMPOID &oid,unsigned long &total,int &count);
	private:
			
		CNetSnmp *cnet;
	};




#ifndef __cplusplus
}
#endif  // extern "C"

#endif  //__BD_NSMP_INTERFACE__


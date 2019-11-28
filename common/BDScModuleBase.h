#ifndef _H_BDSCMODULEBASE
#define _H_BDSCMODULEBASE

#include "common.h"
//#include "Poco/Types.h"
//using namespace Poco;

#pragma pack(push,1)

//监控模块接口数据类型定义
#define MODULE_DATA_LOG                         0x1000  // 日志数据
#define MODULE_DATA_PERFORMANCE             0x1100  // 性能(监控)数据
#define MODULE_DATA_SERVICE                     0x1200  // 业务数据
#define MODULE_DATA_FLOW                        0x1300  // 流量数据
#define MODULE_DATA_NETFLOW                 0x1400  // netflow数据  new in 2017.02.07
// 业务主分类数据
#define MODULE_DATA_LOG_SYSLOG                          0x1001
#define MODULE_DATA_LOG_FILELOG                         0x1002
#define MODULE_DATA_LOG_WMI                             0x1003
#define MODULE_DATA_LOG_SNMPTRAP                        0x1004
#define MODULE_DATA_LOG_SFTP                            0x1005
#define MODULE_DATA_PERFORMANCE_PING                    0x1101
#define MODULE_DATA_PERFORMANCE_SCANPORT                0x1102
#define MODULE_DATA_PERFORMANCE_SNMP                    0x1103
#define MODULE_DATA_PERFORMANCE_INSPECTOR               0x1104
#define MODULE_DATA_PERFORMANCE_MONITOR                 0x1105
#define MODULE_DATA_PERFORMANCE_CONTROLLER          0x1106
#define MODULE_DATA_PERFORMANCE_SNMPCHECK           0x1107
#define MODULE_DATA_PERFORMANCE_NMAPSCAN                0x1108
#define MODULE_DATA_PERFORMANCE_BDSAR               0x1109
#define MODULE_DATA_PERFORMANCE_OPENVAS             0x110A  //new in 2017.8.14
#define MODULE_DATA_PERFORMANCE_DOCKER              0x1110
#define MODULE_DATA_PERFORMANCE_SFTP                0x1111  //new in 2017.5.12
#define MODULE_DATA_PERFORMANCE_THREATINT           0x1112  //new in 2017.7.5
#define MODULE_DATA_PERFORMANCE_INSPECURL               0x1114  //new in 2016.12.27
#define MODULE_DATA_PERFORMANCE_WEAKPASSWD          0x1115  //new in 2017.1.6
#define MODULE_DATA_SERVICE_NETTOPO                     0x1201
#define MODULE_DATA_FLOW_NETFLOW                    0x1202  //new in 2017.01.04
#define MODULE_DATA_FLOW_FLOW                                   0x1302


typedef struct tagModIntfDataTypeEx //模块数据接口类型
{
	int nDataType;       //数据类型
	int nMainCategory;   //主分类
	UInt16 nSubCategory; //子分类
	UInt32 dwPackFlag;   //标识（ENDPACK_FLAG_BEGIN,PACK_FLAG_END)
    tagModIntfDataTypeEx()
    {
        nDataType = 0;
        nMainCategory = 0;
        nSubCategory = 0;
        dwPackFlag = 0;
    }
} modintf_datatype_t,*PModIntfDataType;


#pragma pack(pop)


#if defined(_MSC_VER)
        #define BD_CALLBACK_TYPE 
        #include <windows.h>
#else
    #define BD_CALLBACK_TYPE  
#endif 
//回调函数
typedef bool (BD_CALLBACK_TYPE * pFunc_ReportData)(Poco::UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength);
typedef const void * (BD_CALLBACK_TYPE * pFunc_FetchData)(Poco::UInt32 dwModuleId,const PModIntfDataType pDataType,Poco::UInt32& dwRetLen);

class CScModuleBase
{
public:
        CScModuleBase(void)
        {

        }
        virtual ~CScModuleBase(void)
        {

        }

        virtual bool Init(void) = 0;      //初始化数据，先于Start被调用
        virtual bool Start(void) = 0;     //启动模块
        virtual bool Stop(void) = 0;      //停止模块
        virtual bool IsRunning(void) = 0; //检查模块是否处于运行状态
        virtual UInt32 GetModuleId(void) = 0;
    virtual UInt16 GetModuleVersion(void) = 0;
    virtual string GetModuleName(void) = 0;
        virtual bool StartTask(const PModIntfDataType pDataType,const void * pData) = 0;//开始（下发)任务
        virtual bool StopTask(const PModIntfDataType pDataType,const void * pData) = 0;//停止（取消）任务
        virtual bool SetData(const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength) = 0;//调用方下发数据
        virtual void * GetData(const PModIntfDataType pDataType,Poco::UInt32& dwRetLen) = 0;//调用方获取数据
        virtual void FreeData(void * pData) = 0;//调用方释放获取到的内存
        virtual void SetReportData(pFunc_ReportData pCbReport) = 0;//设置上报数据调用指针
        virtual void SetFetchData(pFunc_FetchData pCbFetch) = 0;//设置获取数据调用指针
};

#endif //_H_BDSCMODULEBASE



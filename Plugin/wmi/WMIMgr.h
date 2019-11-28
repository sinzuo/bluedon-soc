/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    WMIMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _WMI_H_
#define _WMI_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"
#include "WmiIncLin.h"


#define  WMI_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("WMI"), str)
#define  WMI_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("WMI"), str)
#define  WMI_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("WMI"), str)
#define  WMI_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("WMI"), str)

#define  WMI_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("WMI"), #str)
#define  WMI_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("WMI"), #str)
#define  WMI_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("WMI"), #str)
#define  WMI_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("WMI"), #str)



typedef struct BDWmiConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
	char chCustIP[50];
	char chUserName[50];
	char chPasswd[50];
}tag_wmi_config_t;


//WMI信息分类
#define  WMI_OPERATINGSYSTEM  1   //操作系统信息
#define  WMI_PROCESSOR        2   //处理器信息
#define  WMI_COMPUTERSYSTEM   3   //计算内存信息
#define  WMI_1394CONTROLLER   4   //1394控制器信息
#define  WMI_BIOS             5   //Bios信息
#define  WMI_VIDEOCONTROLLER  6   //显卡信息
#define  WMI_BUS              7   //总线信息
#define  WMI_SYSTEMSLOT       8   //系统槽位信息
#define  WMI_PARALLELPORT     9   //端口信息
#define  WMI_PROCESS          10  //进程信息
#define  WMI_DISKDRIVER       11  //硬盘信息
#define  WMI_NETADAPTER       12  //网络适配器信息
#define  WMI_NETADAPTERCONFIG 13  //网络适配器配置信息
#define  WMI_SERVICE          14  //服务信息
#define  WMI_PRODUCT          15  //软件信息
#define  WMI_PERFLOGICALDISK  16  //实时硬盘性能信息
#define  WMI_PERFMEMORY       17  //实时内存性能信息
#define  WMI_PERFPROCESSOR    18  //实时CPU性能信息



class CWMIMgr:public CScModuleBase {
	
public:
	CWMIMgr(const string &strConfigName);
	virtual ~CWMIMgr(void); 

	virtual bool Init(void);      //初始化数据，先于Start被调用
	virtual bool Start(void);     //启动模块
	virtual bool Stop(void);      //停止模块
	virtual bool IsRunning(void); //检查模块是否处于运行状态

	virtual UInt32 GetModuleId(void);
    virtual UInt16 GetModuleVersion(void);
    virtual string GetModuleName(void);

	virtual bool  StartTask(const PModIntfDataType pDataType,const void * pData);     //开始（下发)任务
	virtual bool  StopTask(const PModIntfDataType  pDataType,const void * pData);     //停止（取消）任务
	virtual bool  SetData(const PModIntfDataType   pDataType,const void * pData,Poco::UInt32 dwLength);//调用方下发数据
	virtual void* GetData(const PModIntfDataType   pDataType,Poco::UInt32& dwRetLen); //调用方获取数据
	virtual void  FreeData(void * pData);                     //调用方释放获取到的内存
	virtual void  SetReportData(pFunc_ReportData pCbReport); //设置上报数据调用指针
	virtual void  SetFetchData(pFunc_FetchData pCbFetch);    //设置获取数据调用指针

protected:
    bool LoadConfig(void); //加载配置文件
	void printConfig();    //打印配置信息
	bool Load(void);       //加载业务数据
	
	bool ReportData(const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength); //上报数据（主要为监控日志）
	const char* FetchData(const PModIntfDataType pDataType,Poco::UInt32& dwRetLen);             //获取客户端系统参数

    void GetOperatingSystem(string & strResult);  //操作系统信息
    void GetProcessor(string & strResult);        //处理器信息
    void GetComputerSystem(string & strResult);   //计算内存信息
    void Get1394Controller(string & strResult);   //1394控制器信息   
    void GetBios(string & strResult);             //Bios信息
    void GetVideoController(string & strResult);  //显卡信息
    void GetBus(string & strResult);              //总线信息
    void GetSystemSlot(string & strResult);       //系统槽位信息   
    void GetParallelPort(string & strResult);     //端口信息
    void GetProcess(string & strResult);          //进程信息
    void GetDiskDriver(string & strResult);        //硬盘信息
    void GetNetAdapter(string & strResult);        //网络适配器信息
    void GetNetAdapterConfig(string & strResult);  //网络适配器配置信息
    void GetService(string & strResult);           //服务信息
    void GetProduct(string & strResult);           //软件信息
    void GetPerfLogicalDisk(string & strResult);   //实时硬盘性能信息
    void GetPerfMemory(string & strResult);        //实时内存性能信息
    void GetPerfProcessor(string & strResult);     //操作系统信息

private:
	
	static string fromBase64(const std::string &source);
	static void* OnFetchHandle(void *arg);    //Fetch Data 事件线程处理函数
    static void* OnReportHandle(void *arg);   //Report Data 事件线程处理函数

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;
	bool m_bIsRunning;

	//pthread_t p_thread_fetch;
	//Poco::Mutex      m_mutex_fetch;           //m_setFetch 队列互斥锁
	//list<modintf_datatype_t> m_listFetch;     //Fetch 数据任务队列

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_fetch;
	Poco::Mutex       m_mutex_fetch;

	CWMI *m_wmiPtr;                             
	list<modintf_datatype_t> m_listFetch;       //Fetch 数据任务队列

public:
	tag_wmi_config_t m_wmiConfig;
	
};

#endif //_WMI_H_





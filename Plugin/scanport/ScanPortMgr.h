/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    ScanPortMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SCANPORT_MGR_H_
#define _SCANPORT_MGR_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"


#define  SCANPORT_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SCANPORT"), str)
#define  SCANPORT_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SCANPORT"), str)
#define  SCANPORT_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SCANPORT"), str)
#define  SCANPORT_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SCANPORT"), str)

#define  SCANPORT_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SCANPORT"), #str)
#define  SCANPORT_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SCANPORT"), #str)
#define  SCANPORT_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SCANPORT"), #str)
#define  SCANPORT_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SCANPORT"), #str)


typedef struct BDScanPortConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];          //记录分隔符
	char chFieldSep[2];           //字段分隔符
}tag_scanport_config_t;

typedef struct BDScanPortPolicyEx {
	char chIP[64];
	unsigned short int sin_port_beg; 
	unsigned short int sin_port_end; 
}tag_canport_policy_t;

/*typedef struct BDScanPortThreadPoolEx {
	pthread_t t_id;
	unsigned int t_num;
	bool b_state;
}tag_scanport_threadpool_t;*/


class CScanPortMgr:public CScModuleBase {
	
public:
	CScanPortMgr(const string &strConfigName);
	virtual ~CScanPortMgr(void); 


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
	pthread_t         p_thread_report;
	//Poco::Mutex       m_mutex_report;

	vector<tag_canport_policy_t> m_vecPolicy;  //Fetch 数据任务队列
	//vector<tag_scanport_threadpool_t> m_vecThreadPool; 
	
public:
	tag_scanport_config_t m_pingConfig;

};

#endif //_SCANPORT_MGR_H_





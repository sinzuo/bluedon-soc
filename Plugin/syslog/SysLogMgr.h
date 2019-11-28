/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SysLogMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SYSLOG_MGR_H_
#define _SYSLOG_MGR_H_

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>

#include "common/BDScModuleBase.h"
#include "SysLogUdp.h"
#include "SysLogTcp.h"
#include "config/BDOptions.h"
#include "Poco/Mutex.h"
#include <vector>
#include <set>
#include <list>
#include <string>

//log4xx宏定义
#define  SYSLOG_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SYSLOG"), str);
#define  SYSLOG_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SYSLOG"), str);
#define  SYSLOG_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SYSLOG"), str);
#define  SYSLOG_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SYSLOG"), str);

#define  SYSLOG_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SYSLOG"), #str);
#define  SYSLOG_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SYSLOG"), #str);
#define  SYSLOG_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SYSLOG"), #str);
#define  SYSLOG_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SYSLOG"), #str);


typedef struct BDSyslogConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
	int  nListenPort;
    char chRecordSep[2];        //记录分隔符
    char chFieldSep[2];         //字段分隔符
	char chDelimiter[20];         //分割字符串
    bool bUseUdpRecv;           //是否用udp接收(默认true)
    unsigned int nListCtrl;     //队列长度控制
    unsigned int nBreakCtrl;    //report跳出控制
    unsigned int nSleepTimeMs;  //睡眠时间(毫秒)
	bool nSendtoKafka;  //是否直接发送到kafka
}tag_syslog_config_t;

//udp,tcp方式相同数据变量
class ShareData
{
	
public:
    static Poco::Mutex             m_inputMutex;
    static std::list<std::string>  m_strLogList;
	static string m_strDelimiter;
	static long list_num;

};


class CSysLogMgr:public CScModuleBase {
	
public:
	CSysLogMgr(const string &strConfigName);
	virtual ~CSysLogMgr(void); 

	virtual bool Init(void);      //初始化数据，先于Start被调用
	virtual bool Start(void);     //启动模块
	virtual bool Stop(void);      //停止模块
	virtual bool IsRunning(void); //检查模块是否处于运行状态

	virtual UInt32 GetModuleId(void);  //获取模块id
    virtual UInt16 GetModuleVersion(void);//获取模块版本
    virtual string GetModuleName(void);//获取模块名称

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
	static void* OnSatrtUdpServer(void *arg); //开启UDP监听以及I/O事件检测
    static void* OnSatrtTcpServer(void *arg); //开启TCP监听以及I/O事件检测
	static void* OnFetchHandle(void *arg);    //Fetch Data 事件线程处理函数
    static void* OnReportHandle(void *arg);   //Report Data 事件线程处理函数

private:

    pFunc_ReportData  m_pFuncReportData;
    pFunc_FetchData   m_pFuncFetchData;

    std::string       m_strConfigName;

	//pthread_t p_thread_fetch;
	//Poco::Mutex      m_mutex_fetch;           //m_setFetch 队列互斥锁
	//list<modintf_datatype_t> m_listFetch;     //Fetch 数据任务队列

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
    pthread_t         p_thread_server;
    //Poco::Mutex       m_mutex_report;
    UdpClientService*  pServer;
    TcpClientAcceptor* pTcpServer; //add in 2017.04.25

    bool              m_bIsRunning;
    bool              m_bServerState; //m_bUdpServerState

    std::set<std::string>   m_setPolicy;

public:
    tag_syslog_config_t     m_sysLogConfig;

};

#endif //_SYSLOG_MGR_H_





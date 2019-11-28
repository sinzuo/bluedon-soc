/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SnmpTrapMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SNMPTRAP_MGR_H_
#define _SNMPTRAP_MGR_H_

#include "common/BDScModuleBase.h"
#include "SnmpTrapServer.h"
#include "config/BDOptions.h"
#include <vector>

typedef struct BDSnmpTrapConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
	char chListenPort[20];
    char chRecordSep[2];          //记录分隔符
	char chFieldSep[2];           //字段分隔符
}tag_snmptrap_config_t;


class CSnmpTrapMgr:public CScModuleBase {
	
public:
	CSnmpTrapMgr(const string &strConfigName);
	virtual ~CSnmpTrapMgr(void); 

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
	static void* OnSnmpTrapServer(void *arg); //开启UDP监听以及I/O事件检测
	static void* OnFetchHandle(void *arg);    //Fetch Data 事件线程处理函数
    static void* OnReportHandle(void *arg);   //Report Data 事件线程处理函数

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;

	//pthread_t p_thread_fetch;
	//Poco::Mutex      m_mutex_fetch;           //m_setFetch 队列互斥锁
	//list<modintf_datatype_t> m_listFetch;     //Fetch 数据任务队列

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	pthread_t         p_thread_snmptrap;
	Poco::Mutex       m_mutex_report;
	CSnmpTrapServer* pServer;

	bool m_bIsRunning;
    bool m_bUdpServerState;

	vector<string> m_listPolicy;

public:
	tag_snmptrap_config_t m_snmpTrapConfig;	
	
};

#endif //_SNMPTRAP_MGR_H_




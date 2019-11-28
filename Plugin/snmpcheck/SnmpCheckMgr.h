﻿/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SnmpCheckMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SNMPCHECK_MGR_H_
#define _SNMPCHECK_MGR_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"
#include <vector>
#include <string>
#include <map>
using namespace std;


#define  SNMPCHECK_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SNMPCHECK"), str)
#define  SNMPCHECK_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SNMPCHECK"), str)
#define  SNMPCHECK_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SNMPCHECK"), str)
#define  SNMPCHECK_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SNMPCHECK"), str)

#define  SNMPCHECK_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SNMPCHECK"), #str)
#define  SNMPCHECK_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SNMPCHECK"), #str)
#define  SNMPCHECK_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SNMPCHECK"), #str)
#define  SNMPCHECK_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SNMPCHECK"), #str)


//配置文件结构
typedef struct BDSnmpCheckConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];        //记录分隔符, '|'
    char chFieldSep[2];         //字段分隔符,	'~'
	int nTaskNumPerThread;      //每个线程处理的任务数
}tag_snmpCheck_config_t;

//线程池结构
typedef struct BDSnmpCheckThreadPoolEx {
	pthread_t t_id;
	unsigned int t_num;
	bool b_state;
}tag_snmpCheck_threadpool_t;

//SNMPCHECK插件策略结构
typedef struct BDSnmpCheckPolicyEx {
    BDSnmpCheckPolicyEx()
    {
        vec_options.clear();
        //为何和inspector插件中子任务序号保持一致，这里子任务序号不使用0
        for (int i=1; i <= 4; i++)
        { // 1:网络信息列表 2:安装软件列表 3:服务状态列表 4:设备列表 //(5:磁盘信息列表)
            m_mapFinished[i-1] = false;
            m_mapResult[i-1]   = "";
        }
    }

    vector<string> vec_options;    // 参数列表
    map<int,bool> m_mapFinished;   // 任务完成标志(细化至子任务)
    map<int,string> m_mapResult;   // 任务结果字符串(细化至子任务)
}tag_snmpCheck_policy_t;


class CSnmpCheckMgr:public CScModuleBase {
	
public:
    CSnmpCheckMgr(const string &strConfigName);
    virtual ~CSnmpCheckMgr(void);


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
	virtual void  FreeData(void * pData);                    //调用方释放获取到的内存
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
    static void* OnSnmpCheckHandle(void *arg);     //snmpCheck处理

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;
	bool m_bIsRunning;

    int m_time_interval;
	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	//Poco::Mutex       m_mutex_report;
	bool m_bIsLoaded; // 模块是否加载了

    vector<tag_snmpCheck_policy_t> m_vecPolicy;           // 策略(任务)队列
    vector<tag_snmpCheck_threadpool_t> m_vecThreadPool;   // 线程池
	
	int m_pthreadnum;            // 线程池中预创建线程的数量
	int m_pthreadnum_alive;      // 线程池中活着的线程数量 
	
public:
    tag_snmpCheck_config_t m_snmpCheckConfig;

};

#endif //_SNMPCHECK_MGR_H_





/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    TopuMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _TOPU_MGR_H_
#define _TOPU_MGR_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"
#include "toputask/bd_topu_task.h"


#define  TOPU_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("TOPU"), str)
#define  TOPU_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("TOPU"), str)
#define  TOPU_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("TOPU"), str)
#define  TOPU_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("TOPU"), str)

#define  TOPU_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("TOPU"), #str)
#define  TOPU_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("TOPU"), #str)
#define  TOPU_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("TOPU"), #str)
#define  TOPU_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("TOPU"), #str)

//配置文件结构
typedef struct BDTopuConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];        //记录分隔符
	char chFieldSep[2];         //字段分隔符	
    int nTaskNumPerThread;      //每个线程处理的主机数
}tag_topu_config_t;

//线程池结构
typedef struct BDTopuThreadPoolEx {
	pthread_t t_id;
	unsigned int t_num;
	bool b_state;
}tag_topu_threadpool_t;

//TOPU插件策略结构
typedef struct BDTopuPolicyEx {
	BDTopuPolicyEx() {
		vec_options.clear();
        for (int i=0; i < 2; i++) { // 0:拓扑 1:设备
            m_mapFinished[i] = false;
            m_mapResult[i]   = "";
		}
	}
	vector<string> vec_options;          // 参数字符串
	map<int,bool> m_mapFinished;         // 任务执行标志(细化至子任务)
	map<int,string> m_mapResult;         // 任务结果字符串(细化至子任务)
}tag_topu_policy_t;


class CTopuMgr:public CScModuleBase {
	
public:
	CTopuMgr(const string &strConfigName);
	virtual ~CTopuMgr(void); 


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
    static void* OnTopuHandle(void *arg);     //topu处理
    static void* OngethostHandle(void *arg);   //获取设备信息线程

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;
	bool m_bIsRunning;

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	//Poco::Mutex       m_mutex_report;
 
    vector<tag_topu_policy_t> m_vecPolicy;           // 策略(任务)队列
	vector<tag_topu_threadpool_t> m_vecThreadPool;   // 线程池
	
	int m_pthreadnum;            // 线程池中预创建线程的数量
    int m_pthreadnum_alive;      // 线程池中活着的线程数量


    tag_topu_policy_t m_topu_vecPolicy;           // 拓扑任务

    /*主机线程变量*/
    bool m_host_bIsRunning;

    /*主机信息是否发送完毕*/
    //bool m_host_get;

    /*控制插件状态*/
    //bool m_plugin_bIsRunning;

    vector<tag_topu_policy_t> m_host_vecPolicy;           // 获取主机信息任务队列
    vector<tag_topu_threadpool_t> m_host_vecThreadPool;   // 获取主机信息线程池

    int m_host_pthreadnum;            // 获取主机信息线程池中预创建线程的数量
    int m_host_pthreadnum_alive;      // 获取主机信息线程池中活着的线程数量

    vector<L2_TOPO_INFO> l2_topuinfo_list;  //保存主机设备拓扑信息
    vector<HOSTINFO> hostinfo_vec;  //保存主机设备拓扑信息


    SNMP_CONN_INFO m_conn_info;
public:
	tag_topu_config_t m_topuConfig;

};

#endif //_TOPU_MGR_H_





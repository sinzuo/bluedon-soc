/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SftpMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SFTP_MGR_H_
#define _SFTP_MGR_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"
#include <vector>

#define  SFTP_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SFTP"), str)
#define  SFTP_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SFTP"), str)
#define  SFTP_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SFTP"), str)
#define  SFTP_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SFTP"), str)

#define  SFTP_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SFTP"), #str)
#define  SFTP_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SFTP"), #str)
#define  SFTP_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SFTP"), #str)
#define  SFTP_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SFTP"), #str)

//配置文件结构
typedef struct BDSftpConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];        //记录分隔符
	char chFieldSep[2];         //字段分隔符	
	int nTaskNumPerThread;      //每个线程处理的任务数
    int    nReadLineNum;        //一次性读取的行数
}tag_sftp_config_t;

//线程池结构
typedef struct BDSftpThreadPoolEx {
	pthread_t t_id;
	unsigned int t_num;
	bool b_state;
}tag_sftp_threadpool_t;

//SFTP插件策略结构
typedef struct BDSftpPolicyEx {
	BDSftpPolicyEx() {
        vec_options.clear();
        vec_log.clear();
        bSendFalg = false;              // 默认无日志发送，即 temp.vec_log为空
        m_mapFinished[0] = false;
//         m_mapResult[0] = "";
	}
	vector<string> vec_options;          // 参数字符串
	map<int,bool> m_mapFinished;         // 任务执行标志(细化至子任务)
//	map<int,string> m_mapResult;         // 任务结果字符串(细化至子任务)
    string strSourceIP;       // 原设备IP
    string strUser;            //上报用户
    vector<string>  vec_log;  // 待发送日志
    bool bSendFalg;           // 日志发送标志 true:可以发送(vec_log非空)
}tag_sftp_policy_t;

class CSftp:public CScModuleBase {
	
public:
	CSftp(const string &strConfigName);
	virtual ~CSftp(void); 


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

    void ReadFileLog(void *arg, int thread_num, string fileName);
	
	bool ReportData(const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength); //上报数据（主要为监控日志）
	const char* FetchData(const PModIntfDataType pDataType,Poco::UInt32& dwRetLen);             //获取客户端系统参数


private:
	
	static string fromBase64(const std::string &source);
	static void* OnFetchHandle(void *arg);    //Fetch Data 事件线程处理函数
    static void* OnReportHandle(void *arg);   //Report Data 事件线程处理函数
    static void* OnSftpHandle(void *arg);     //管控处理

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;
	bool m_bIsRunning;

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	//Poco::Mutex       m_mutex_report;
	bool m_bIsLoaded; // 模块是否加载了

	vector<tag_sftp_policy_t> m_vecPolicy;           // 策略(任务)队列
	vector<tag_sftp_threadpool_t> m_vecThreadPool;   // 线程池
	
	int m_pthreadnum;            // 线程池中预创建线程的数量
	int m_pthreadnum_alive;      // 线程池中活着的线程数量 
	
public:
	tag_sftp_config_t m_sftpConfig;

};

#endif //_SFTP_MGR_H_





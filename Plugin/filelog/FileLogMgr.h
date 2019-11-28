/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    FileLogMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _FILELOG_MGR_H_
#define _FILELOG_MGR_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

#define  FILELOG_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("FILELOG"), str);
#define  FILELOG_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("FILELOG"), str);
#define  FILELOG_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("FILELOG"), str);
#define  FILELOG_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("FILELOG"), str);

#define  FILELOG_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("FILELOG"), #str);
#define  FILELOG_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("FILELOG"), #str);
#define  FILELOG_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("FILELOG"), #str);
#define  FILELOG_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("FILELOG"), #str);

//配置文件
typedef struct BDFileLogConfigEx {
    char   chLog4File[100];
    int    nModuleId;
	char   chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];        //记录分隔符
	char chFieldSep[2];         //字段分隔符		
    int    nReadLineNum;        //一次性读取的行数    
	int    nTaskNumPerThread;   //每个线程处理的目录数
    bool   bLargeLog;           //是否是大文件
}tag_filelog_config_t;

//日志文件结构
typedef struct BDFileLogInfoEx {
	string   strFilename;
	time_t mtime;    //最后一次文件修改时间
	bool operator < (const BDFileLogInfoEx &right) 
	{
	   return mtime < right.mtime;
	}
}tag_filelog_info_t;

//线程池结构
typedef struct BDFileLogThreadPoolEx {
	pthread_t t_id;
	unsigned int t_num;
	bool b_state;
}tag_filelog_threadpool_t;

//FILELOG插件策略结构
typedef struct BDFileLogPolicyEx {
	BDFileLogPolicyEx() {
		dirPtr = NULL;
		vec_log.clear();
		bSendFalg = false;  // 默认无日志发送，即 temp.vec_log为空 
	}

	string strDirPath;        // 目录名称
	DIR * dirPtr;    
	string strSourceIP;       // 原设备IP
	string strAppName;        // 应用名称
	vector<string>  vec_log;  // 待发送日志
	bool bSendFalg;           // 日志发送标志 true:可以发送(vec_log非空)     
}tag_filelog_policy_t,*tag_filelog_policy_p ;


class CFileLogMgr:public CScModuleBase {
	
public:
	CFileLogMgr(const string &strConfigName);
	virtual ~CFileLogMgr(void); 

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

	void GetFileName(tag_filelog_policy_p policyPtr, list<tag_filelog_info_t> &filenamelist);   //获取待处理文件
	void Backup(const string &strFileName);  //备份文件
    bool InitPosFile(const tag_filelog_policy_t &policy, list<tag_filelog_info_t> &filenamelist);
    void MarkPos(std::fstream &fPosFile, std::streamoff markPos, const string &strFileName, long long posCurrent, long long fileSize);  //标记位置
    bool ReadPos(std::fstream &fPosFile, std::streamoff &markPos, const string &strFileName, long long &posCurrent, long long &fileSize);  //获取标记位置
	
	bool ReportData(const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength); //上报数据（主要为监控日志）
	const char* FetchData(const PModIntfDataType pDataType,Poco::UInt32& dwRetLen);             //获取客户端系统参数

private:
	
	static string fromBase64(const std::string &source);
	static void* OnSatrtUdpServer(void *arg); //开启UDP监听以及I/O事件检测
	static void* OnFetchHandle(void *arg);    //Fetch Data 事件线程处理函数
    static void* OnReportHandle(void *arg);   //Report Data 事件线程处理函数
    static void* OnFilelogHandle(void *arg);  //文件日志获取处理线程

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;

	//pthread_t p_thread_fetch;
	//Poco::Mutex      m_mutex_fetch;           //m_setFetch 队列互斥锁
	//list<modintf_datatype_t> m_listFetch;     //Fetch 数据任务队列

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	Poco::Mutex       m_mutex_report;
	bool m_bIsLoaded; // 模块是否加载了
	bool m_bIsRunning;

    vector<tag_filelog_policy_t> m_vecPolicy;		   // 策略(任务)队列
    vector<tag_filelog_threadpool_t> m_vecThreadPool;  // 线程池
    int m_pthreadnum;			                       // 线程池中预创建线程的数量
    int m_pthreadnum_alive; 	                       // 线程池中活着的线程数量 

public:
	tag_filelog_config_t m_fileLogConfig;
	
	//list<tag_filelog_info_t> m_listFileInfo;
};

#endif //_FILELOG_MGR_H_




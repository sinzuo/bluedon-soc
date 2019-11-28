
/** ************************************************
* COPYRIGHT NOTICE
* Copyright (c) 2017,      BLUEDON
* All rights reserved.
*
* @file    BDScModuleApp.h
* @brief   传感器主程序, 负责与管理引擎和分析引擎的通信
*
*
* @version 5.1
* @author  xxx
* @date    YYYY年MM月DD日
*
* 修订说明：最初版本
* *************************************************/


#ifndef  _BDSCMODULEAPP_H_
#define  _BDSCMODULEAPP_H_

#include <sstream>
#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
#include "Poco/Mutex.h"
#include "Poco/Thread.h"
#include "Poco/DeflatingStream.h"
#include "Poco/InflatingStream.h"

#include "BDScModuleMgr.h"
#include "config/BDOptions.h"
#include "probuf/socmsg.pb.h"
#include "BDBaseSysInfo.h"

using namespace bd_soc_protobuf;


typedef list<bd_request_t>::iterator Bd_Request_Itor;

#define MUTEX_MS_REQ 0
//设置protobuf消息体，并存放到map中
#define PB_SEND_MSG_INI(var,desc,type,type1)\
    if (var == NULL) {\
        var = new Message();\
        var->set_desc(#desc);\
        var->set_service_no(type);\
        var->set_serial_no(0);\
        m_MessageObj[type1] = var;\
    }


/**
 * @brief   传感器工作模块
 * @class   CBDScModuleApp
 * @author
 * @date    2016年4月
 */
class CBDScModuleApp {
private: //singleton

    CBDScModuleApp();
    CBDScModuleApp(const CBDScModuleApp&);
    ~CBDScModuleApp();
public:
    inline static CBDScModuleApp* inistance(){ return &m_App; }
    void Daemon();                              // make daemon
    int CreatePidFile(const char * pidfile);
    void PrintUsage();                          // print simple info
    void CommondParse(char argc, char **grgv);  // parse commandline paramters
	//void InitLog();
    int Initlize();	     //初始化入口
	int InitOptions();  //initlize log and configure 
	int InitModuleMgr();//模块初始化
	int InitProBuf();   //初始化probuf
	int InitZMQ();      //初始化zmq
	int InitBaseSysInfo(); //初始化zmq
//	int InitKAFKA();    //初始化kafka
    //int initsignal(); 
    void printConfig();  //打印配置
    
    int RunProcess(); 
	bool  Sender(const string &str, const PModIntfDataType pDataType);

private:

	// get CBDScModuleMgr obj
	inline CBDScModuleMgr *GetBDScModuleMgr() { 
	    return CBDScModuleMgr::instance();
	}
	// get CBDOptions obj
    inline CBDOptions *GetCBOptions() { 
	    return CBDOptions::instance();	
	}
	// get CBDBaseSysInfo obj
    inline CBDBaseSysInfo *GetCBDBaseSysInfo() { 
	    return CBDBaseSysInfo::instance();	
	}	
	// pop element from list
	inline void RequestPop(int mutex_type) {
		m_mutex[mutex_type].lock();
		m_listreq.pop_front();
		m_mutex[mutex_type].unlock();	
	}

private:

    static void* HeartBeatSend(void *arg);//发送心跳
    static void* HeartBeatRecv(void *arg);//接收心跳
    static void* Reader(void *arg); //接收策略
    static void* Worker(void *arg);	//解析策略

    int TimeDiff();//获取时间差
    void EnCodeData(string &strContent, bool isEcpt, Message_enptypeNo eType, bool isCpss, Message_comtypeNo cType);
    void DeCodeData(string &strContent, bool isEcpt, Message_enptypeNo eType, bool isCpss, Message_comtypeNo cType);

private:
    static CBDScModuleApp m_App;  // singleton
	bool m_bdaemon;               // Demon flag
	string m_strcfgname;          // ConfigureName (include path)
	string m_strApppath;          // Application Path

	void *m_zmq_context;          // ZMQ Contex
	void *m_zmq_server;           // ZMQ Server Socket for MS
	void *m_zmq_requester;        // ZMQ Client Socket for MS
	void *m_zmq_responseterA;     // ZMQ Client Socket for AS Log
	void *m_zmq_responseterB;     // ZMQ Client Socket for AS Performance
    void *m_zmq_responseterC;     // ZMQ Client Socket for AS Service
    void *m_zmq_responseterD;     // ZMQ Client Socket for AS Flow
    void *m_zmq_responseterE;     // ZMQ Client Socket for AS netFlow
	Message * m_pbHeartbeatWritePtr;  // Google PB for sending to MS as client
	Message * m_pbHeartbeatReadPtr;   // Google PB for reading from MS as client
	Message * m_pbServerPtr;          // Google PB for reading from MS as server

	Message * m_pbASWritePtr_syslog;      // Google PB for sending syslog to AS
	Message * m_pbASWritePtr_filelog;     // Google PB for sending filelog to AS
	Message * m_pbASWritePtr_snmptrap;    // Google PB for sending snmptrap to AS
	Message * m_pbASWritePtr_ping;        // Google PB for sending ping to AS
	Message * m_pbASWritePtr_scanport;    // Google PB for sending scanport to AS
	Message * m_pbASWritePtr_inspector;   // Google PB for sending inspector to AS
	Message * m_pbASWritePtr_monitor;     // Google PB for sending monitor to AS
	Message * m_pbASWritePtr_controller;  // Google PB for sending controller to AS
    Message * m_pbASWritePtr_snmpcheck;  // Google PB for sending snmpcheck to AS
    Message * m_pbASWritePtr_topu;  // Google PB for sending topu to AS
    Message * m_pbASWritePtr_wmi;  // Google PB for sending wmi to AS
    Message * m_pbASWritePtr_nmapscan;  // Google PB for sending topu to AS
    Message * m_pbASWritePtr_bdsar;  // Google PB for sending bdsar to AS //added in 2017.2.27
    Message * m_pbASWritePtr_openvas;  // Google PB for sending openvas to AS //added in 2017.8.14
    Message * m_pbASWritePtr_sftp;  // Google PB for sending sftp to AS //added in 2017.5.16
    Message * m_pbASWritePtr_threatint;  // Google PB for sending threatint to AS //added in 2017.7.5
    Message * m_pbASWritePtr_inspecUrl;  // Google PB for sending inspecUrl to AS //added in 2016.12.28
    Message * m_pbASWritePtr_weakpasswd;  // Google PB for sending weakpasswd to AS //added in 2016.1.6
    Message * m_pbASWritePtr_netflow;  // Google PB for sending netflow to AS //added in 2016.1.9
    Message * m_pbASWritePtr_flow;  // Google PB for sending flow to AS //added in 2016.1.10
    list<bd_request_t> m_listreq;  // request list
    Poco::Mutex m_mutex[2];        // 0:m_listreq : 
    pthread_t m_pthreads[4];       // 0:heartbeatsend 1:heartbeatrecv 2:reader 3:worker
	int m_nTimeSleepHeartBeat;     // sleep time for heartbeat
	int m_nTimeMonitorHeartBeat;   // frequency of reporting monitor info
	bool m_registered;             // flag for registering
    bool m_isCompressed;             // flag for whether data need to compress
    bool m_isEncrypted;              // flag for whether data need to encrypt
    Message_comtypeNo  m_nCompressType;
    Message_enptypeNo  m_nEncryptType;
    static unsigned int m_nLog;    // number of logs for sending to AS

    map<int,Message *> m_MessageObj; //set map for message
    map<int,void *>    m_ZmqObj;    //set Zmq Object for message
	
	tag_proxy_config_pt m_agentConfigPtr; // config
 	
};

#endif //_BDSCMODULEAPP_H_


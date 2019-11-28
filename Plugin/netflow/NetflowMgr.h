/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    NetflowMgr.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _NETFLOW_MGR_H_
#define _NETFLOW_MGR_H_

#include "common/BDScModuleBase.h"
#include "NetflowUdp.h"
#include "config/BDOptions.h"
#include <vector>
#include <map>
//#include "GeoIP.h"
//#include "GeoIPCity.h"


typedef struct BDNetflowConfigEx {
    char chLog4File[100];
    int nModuleId;
    char chModuleName[20];
    UInt16 wModuleVersion;
    int  nListenPort;
    char chRecordSep[2];          //记录分隔符
    char chFieldSep[2];           //字段分隔符
    unsigned int nListCtrl;        // 队列长度控制
    unsigned int nBreakCtrl;    //report跳出控制
    unsigned int nSleepTimeMs;  //睡眠时间(毫秒)
    char Geoipdatafile[100];    //Geoip file
}tag_netflow_config_t;


class CNetflowMgr:public CScModuleBase {

public:
    CNetflowMgr(const string &strConfigName);
    virtual ~CNetflowMgr(void);

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

    static int loadingstat(string srcip, string dstip); //流量是判断上传下载;
    static bool isInnerIP (string IP);  //判断是否内网ip
    static string fromBase64(const std::string &source);
    static void* OnStartUdpServer(void *arg); //开启UDP监听以及I/O事件检测
    static void* OnFetchHandle(void *arg);    //Fetch Data 事件线程处理函数
    static void* OnReportHandle(void *arg);   //Report Data 事件线程处理函数

private:

//	GeoIP *gi; //获取ip地址经纬度信息

    pFunc_ReportData m_pFuncReportData;
    pFunc_FetchData  m_pFuncFetchData;

    string m_strConfigName;

    //pthread_t p_thread_fetch;
    //Poco::Mutex      m_mutex_fetch;           //m_setFetch 队列互斥锁
    //list<modintf_datatype_t> m_listFetch;     //Fetch 数据任务队列

    Poco::FastMutex   m_ModMutex;
    pthread_t         p_thread_report;
    pthread_t         p_thread_udp;
    Poco::Mutex       m_mutex_report;
    netUdpClientService* pServer;

    bool m_bIsRunning;
	bool m_bUdpServerState;

    vector<string> m_vecPolicy;
    vector<string> m_vecfilter_srcip;
    vector<string> m_vecfilter_dstip;
    vector<string> m_vecfilter_prot;

    map<int,string> mask_str;
    map<int,string> prot_str;
    map<int,string> prot7_str;

public:
    tag_netflow_config_t m_NetflowConfig;

};

#endif //_NETFLOW_MGR_H_





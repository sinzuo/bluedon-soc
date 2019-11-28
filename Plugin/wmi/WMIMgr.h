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
 * @date    2017��08��03��
 *
 * �޶�˵��������汾
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


//WMI��Ϣ����
#define  WMI_OPERATINGSYSTEM  1   //����ϵͳ��Ϣ
#define  WMI_PROCESSOR        2   //��������Ϣ
#define  WMI_COMPUTERSYSTEM   3   //�����ڴ���Ϣ
#define  WMI_1394CONTROLLER   4   //1394��������Ϣ
#define  WMI_BIOS             5   //Bios��Ϣ
#define  WMI_VIDEOCONTROLLER  6   //�Կ���Ϣ
#define  WMI_BUS              7   //������Ϣ
#define  WMI_SYSTEMSLOT       8   //ϵͳ��λ��Ϣ
#define  WMI_PARALLELPORT     9   //�˿���Ϣ
#define  WMI_PROCESS          10  //������Ϣ
#define  WMI_DISKDRIVER       11  //Ӳ����Ϣ
#define  WMI_NETADAPTER       12  //������������Ϣ
#define  WMI_NETADAPTERCONFIG 13  //����������������Ϣ
#define  WMI_SERVICE          14  //������Ϣ
#define  WMI_PRODUCT          15  //�����Ϣ
#define  WMI_PERFLOGICALDISK  16  //ʵʱӲ��������Ϣ
#define  WMI_PERFMEMORY       17  //ʵʱ�ڴ�������Ϣ
#define  WMI_PERFPROCESSOR    18  //ʵʱCPU������Ϣ



class CWMIMgr:public CScModuleBase {
	
public:
	CWMIMgr(const string &strConfigName);
	virtual ~CWMIMgr(void); 

	virtual bool Init(void);      //��ʼ�����ݣ�����Start������
	virtual bool Start(void);     //����ģ��
	virtual bool Stop(void);      //ֹͣģ��
	virtual bool IsRunning(void); //���ģ���Ƿ�������״̬

	virtual UInt32 GetModuleId(void);
    virtual UInt16 GetModuleVersion(void);
    virtual string GetModuleName(void);

	virtual bool  StartTask(const PModIntfDataType pDataType,const void * pData);     //��ʼ���·�)����
	virtual bool  StopTask(const PModIntfDataType  pDataType,const void * pData);     //ֹͣ��ȡ��������
	virtual bool  SetData(const PModIntfDataType   pDataType,const void * pData,Poco::UInt32 dwLength);//���÷��·�����
	virtual void* GetData(const PModIntfDataType   pDataType,Poco::UInt32& dwRetLen); //���÷���ȡ����
	virtual void  FreeData(void * pData);                     //���÷��ͷŻ�ȡ�����ڴ�
	virtual void  SetReportData(pFunc_ReportData pCbReport); //�����ϱ����ݵ���ָ��
	virtual void  SetFetchData(pFunc_FetchData pCbFetch);    //���û�ȡ���ݵ���ָ��

protected:
    bool LoadConfig(void); //���������ļ�
	void printConfig();    //��ӡ������Ϣ
	bool Load(void);       //����ҵ������
	
	bool ReportData(const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength); //�ϱ����ݣ���ҪΪ�����־��
	const char* FetchData(const PModIntfDataType pDataType,Poco::UInt32& dwRetLen);             //��ȡ�ͻ���ϵͳ����

    void GetOperatingSystem(string & strResult);  //����ϵͳ��Ϣ
    void GetProcessor(string & strResult);        //��������Ϣ
    void GetComputerSystem(string & strResult);   //�����ڴ���Ϣ
    void Get1394Controller(string & strResult);   //1394��������Ϣ   
    void GetBios(string & strResult);             //Bios��Ϣ
    void GetVideoController(string & strResult);  //�Կ���Ϣ
    void GetBus(string & strResult);              //������Ϣ
    void GetSystemSlot(string & strResult);       //ϵͳ��λ��Ϣ   
    void GetParallelPort(string & strResult);     //�˿���Ϣ
    void GetProcess(string & strResult);          //������Ϣ
    void GetDiskDriver(string & strResult);        //Ӳ����Ϣ
    void GetNetAdapter(string & strResult);        //������������Ϣ
    void GetNetAdapterConfig(string & strResult);  //����������������Ϣ
    void GetService(string & strResult);           //������Ϣ
    void GetProduct(string & strResult);           //�����Ϣ
    void GetPerfLogicalDisk(string & strResult);   //ʵʱӲ��������Ϣ
    void GetPerfMemory(string & strResult);        //ʵʱ�ڴ�������Ϣ
    void GetPerfProcessor(string & strResult);     //����ϵͳ��Ϣ

private:
	
	static string fromBase64(const std::string &source);
	static void* OnFetchHandle(void *arg);    //Fetch Data �¼��̴߳�����
    static void* OnReportHandle(void *arg);   //Report Data �¼��̴߳�����

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;
	bool m_bIsRunning;

	//pthread_t p_thread_fetch;
	//Poco::Mutex      m_mutex_fetch;           //m_setFetch ���л�����
	//list<modintf_datatype_t> m_listFetch;     //Fetch �����������

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_fetch;
	Poco::Mutex       m_mutex_fetch;

	CWMI *m_wmiPtr;                             
	list<modintf_datatype_t> m_listFetch;       //Fetch �����������

public:
	tag_wmi_config_t m_wmiConfig;
	
};

#endif //_WMI_H_





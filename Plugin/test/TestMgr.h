#ifndef _TEST_MGR_H_
#define _TEST_MGR_H_

#include "common/BDScModuleBase.h"
#include "TestUdp.h"
#include "config/BDOptions.h"
#include <vector>

typedef struct BDTestConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
	int  nListenPort;
    char chRecordSep[2];          //��¼�ָ���
	char chFieldSep[2];           //�ֶηָ���
}tag_test_config_t;


class CTestMgr:public CScModuleBase {
	
public:
	CTestMgr(const string &strConfigName);
	virtual ~CTestMgr(void); 

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

private:
	
	static string fromBase64(const std::string &source);
	static void* OnSatrtUdpServer(void *arg); //����UDP�����Լ�I/O�¼����
	static void* OnFetchHandle(void *arg);    //Fetch Data �¼��̴߳�����
    static void* OnReportHandle(void *arg);   //Report Data �¼��̴߳�����

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;

	//pthread_t p_thread_fetch;
	//Poco::Mutex      m_mutex_fetch;           //m_setFetch ���л�����
	//list<modintf_datatype_t> m_listFetch;     //Fetch �����������

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	pthread_t         p_thread_udp;
	Poco::Mutex       m_mutex_report;
	TestUdpClientService* pServer;

	bool m_bIsRunning;

	vector<string> m_vecPolicy;

public:
	tag_test_config_t m_testConfig;
	
};

#endif //_TEST_MGR_H_





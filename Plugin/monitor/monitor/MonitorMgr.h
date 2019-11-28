#ifndef _MONITOR_MGR_H_
#define _MONITOR_MGR_H_

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"

#define  MONITOR_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("MONITOR"), str)
#define  MONITOR_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("MONITOR"), str)
#define  MONITOR_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("MONITOR"), str)
#define  MONITOR_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("MONITOR"), str)

#define  MONITOR_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("MONITOR"), #str)
#define  MONITOR_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("MONITOR"), #str)
#define  MONITOR_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("MONITOR"), #str)
#define  MONITOR_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("MONITOR"), #str)

//�����ļ��ṹ
typedef struct BDMonitorConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];        //��¼�ָ���
	char chFieldSep[2];         //�ֶηָ���	
	int nTaskNumPerThread;      //ÿ���̴߳����������
}tag_monitor_config_t;

//�̳߳ؽṹ
typedef struct BDMonitorThreadPoolEx {
	pthread_t t_id;
	unsigned int t_num;
	bool b_state;
}tag_monitor_threadpool_t;

//MONITOR������Խṹ
typedef struct BDMonitorPolicyEx {
	BDMonitorPolicyEx() {
		vec_options.clear();
		//Ϊ�κ�inspector�������������ű���һ�£�������������Ų�ʹ��0
		for (int i=1; i <= 4; i++) { // 1:cpuʹ���� 2:�ڴ�ʹ���� 3:����ʹ���� 4:����״̬
			m_mapFinished[i-1] = false;
			m_mapResult[i-1]   = "";
		}
	}
	vector<string> vec_options;          // �����ַ���
	map<int,bool> m_mapFinished;         // ����ִ�б�־(ϸ����������)
	map<int,string> m_mapResult;         // �������ַ���(ϸ����������)
}tag_monitor_policy_t;

class CMonitorMgr:public CScModuleBase {
	
public:
	CMonitorMgr(const string &strConfigName);
	virtual ~CMonitorMgr(void); 


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
	virtual void  FreeData(void * pData);                    //���÷��ͷŻ�ȡ�����ڴ�
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
	static void* OnFetchHandle(void *arg);    //Fetch Data �¼��̴߳�����
    static void* OnReportHandle(void *arg);   //Report Data �¼��̴߳�����
    static void* OnMonitorHandle(void *arg);     //ping����

private:

	pFunc_ReportData m_pFuncReportData;
	pFunc_FetchData  m_pFuncFetchData;

	string m_strConfigName;
	bool m_bIsRunning;

	Poco::FastMutex   m_ModMutex;
	pthread_t         p_thread_report;
	//Poco::Mutex       m_mutex_report;

	vector<tag_monitor_policy_t> m_vecPolicy;           // ����(����)����
	vector<tag_monitor_threadpool_t> m_vecThreadPool;   // �̳߳�
	
	int m_pthreadnum;            // �̳߳���Ԥ�����̵߳�����
	int m_pthreadnum_alive;      // �̳߳��л��ŵ��߳����� 
	
public:
	tag_monitor_config_t m_monitorConfig;

};

#endif //_MONITOR_MGR_H_






#include "MonitorMgr.h"
#include "utils/bdstring.h"
#include "task/bd_task.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CMonitorMgr::CMonitorMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_monitorConfig,0,sizeof(m_monitorConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CMonitorMgr::~CMonitorMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//��ʼ�����ݣ�����Start������
bool CMonitorMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_monitorConfig.chLog4File) > 0 ) {
	    if(access(m_monitorConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_monitorConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    MONITOR_INFO_S( CMonitorMgr::Init() successed...);
	printConfig();

    return true;	
}

bool CMonitorMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CMonitorMgr::GetModuleId(void) 
{
	return m_monitorConfig.nModuleId;
}
UInt16 CMonitorMgr::GetModuleVersion(void) 
{
	return m_monitorConfig.wModuleVersion;
}
string CMonitorMgr::GetModuleName(void) 
{
	return m_monitorConfig.chModuleName;
}

//��ʼ���·�)����
bool CMonitorMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//ֹͣ��ȡ��������
bool CMonitorMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CMonitorMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "MONITOR LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "MONITOR LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_monitorConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_monitorConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_monitorConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_monitorConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_monitorConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_monitorConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_monitorConfig.nTaskNumPerThread);
		if( m_monitorConfig.nTaskNumPerThread <= 0 ) {
			m_monitorConfig.nTaskNumPerThread = 5;  // Ĭ��ÿ���̴߳���5��IP
		}
	}
	catch (NotFoundException& exc ) {
		cerr << "MONITOR LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "MONITOR LoadConfig:" << exc.displayText() << endl;	
		return false;
	}
    return true;
}

void CMonitorMgr::printConfig() {
	MONITOR_INFO_S(<=========netservice configure information=========>);
	MONITOR_INFO_V(" log4configfile=>    ["+string(m_monitorConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_monitorConfig.nModuleId);
	MONITOR_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"))
	MONITOR_INFO_V(" modulename=>        ["+string(m_monitorConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_monitorConfig.wModuleVersion);
	MONITOR_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	MONITOR_INFO_V(" record_separator=>  ["+string(m_monitorConfig.chRecordSep)+string("]"));
	MONITOR_INFO_V(" field_separator=>   ["+string(m_monitorConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_monitorConfig.nTaskNumPerThread);
	MONITOR_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
	MONITOR_INFO_S(<=========netservice configure information=========>);
}

bool CMonitorMgr::Load(void)
{
	return true;
}

//���÷��·�����
bool CMonitorMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
	vector<string> vec_task_unit;
	string strRsep = m_monitorConfig.chRecordSep;
	string_split(strPolicy,strRsep,vec_task_unit);
	
	MONITOR_INFO_S(<=======MONITOR PlOCY=======>);
	int nNum = vec_task_unit.size();  // ���IP����
    for( int i=0; i<nNum; i++) {
		MONITOR_INFO_V(" MONITOR POlOCY =>" + vec_task_unit[i]);		
		tag_monitor_policy_t tag_policy;
		vector<string> vec_temp;
		string strFsep = m_monitorConfig.chFieldSep;
		// IP~���������(BDSEC~�û���~����~URL)
		string_split(vec_task_unit[i],strFsep,vec_temp);
		
		if( vec_temp[1] != "BDSEC" && vec_temp.size() != 5) {
			MONITOR_ERROR_S(Invalid polocy string...);
			return false;
		} else {
			tag_policy.vec_options = vec_temp;
		}
        m_vecPolicy.push_back(tag_policy);
    }
	MONITOR_INFO_S(<=======MONITOR PlOCY=======>);	
	
	m_pthreadnum = m_vecPolicy.size()/m_monitorConfig.nTaskNumPerThread;
	if ( m_pthreadnum == 0 ) m_pthreadnum = 1;
	m_vecThreadPool.reserve(m_pthreadnum);   // ��ǰ�����̳߳ؿռ�
	
	return true;
}

//���÷���ȡ����
void * CMonitorMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//���÷��ͷŻ�ȡ�����ڴ�
void CMonitorMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CMonitorMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CMonitorMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CMonitorMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_monitorConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CMonitorMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//����ģ��
bool CMonitorMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
	//string str = "172.16.2.144~BDSEC~admin~888888~https://172.16.2.144:441/cgi-bin/admin/bdsec_daemon";
	//SetData(NULL,&str,str.length());
 	
    if (! m_bIsRunning ) {
		for (int i=0; i<m_pthreadnum; i++) {
			tag_monitor_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
			if (pthread_create(&(m_vecThreadPool[i].t_id), NULL, OnMonitorHandle, this) != 0) {
				MONITOR_ERROR_S(Create OnMonitorHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10���룬�����߳���ҪһЩִ��ʱ��
			m_pthreadnum_alive++;
			MONITOR_INFO_S(Create OnMonitorHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, NULL, OnReportHandle, this) != 0) {
			MONITOR_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1���� 
		MONITOR_INFO_S(Create OnReportHandle Thread successed!);	
	}	
	MONITOR_INFO_S(CMonitorMgr::Start successed!);
	return true;
}

//ֹͣģ��
bool CMonitorMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}	
	MONITOR_INFO_S(CMonitorMgr::Stop successed!);
    return true;
}

void *CMonitorMgr::OnFetchHandle(void *arg) {
	return NULL;
}

// �ϱ���������
void *CMonitorMgr::OnReportHandle(void *arg)
{
	CMonitorMgr *cThis = reinterpret_cast<CMonitorMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //��ʱ�̳߳��������߳��Ѿ��������
		}
        vector<tag_monitor_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_MONITOR;	
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i < nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // �������	
					if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
						MONITOR_DEBUG_S(Report data failed!);
					} else {
						MONITOR_DEBUG_S(Report data success!);
					}
					iter_temp->m_mapFinished[i] = false;  // �ɹ���ʧ���´ζ�����ִ������
				}	
            }
        }		
		if(cThis->m_pthreadnum_alive == 0 ) break; //�����̳߳�ȫ���˳������˳����߳�	
		if(cThis->m_bIsRunning) sleep(1); // ����һֱɨ��
    }
	cThis->m_bIsRunning = false;	
 	MONITOR_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}

void* CMonitorMgr::OnMonitorHandle(void *arg)
{
	CMonitorMgr *cThis = reinterpret_cast<CMonitorMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // ��ǰ�߳�num��
	cThis->m_vecThreadPool[thread_num].b_state = true;   // �߳�״̬�޸�Ϊ����
	int nPolocyNum = cThis->m_vecPolicy.size();
	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {		
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
				 
                vector<string> v_options_tmp = cThis->m_vecPolicy[i].vec_options;
				v_options_tmp.erase(v_options_tmp.begin()); // ʹ����������CTask�����ʽ
				CTask MonitorTask(v_options_tmp);		 

				if ( !cThis->m_vecPolicy[i].m_mapFinished[0] ) { // cpu		
				    MONITOR_INFO_V(string("Ready to get cpu rate from ") + cThis->m_vecPolicy[i].vec_options[0]);
                    int nCpu;                
					char chCpu[4] = {0};					
                    if ( !MonitorTask.getCpuUsage(nCpu)) {
						MONITOR_ERROR_S(Get CPU rate failed!);
                    } else {
       					sprintf(chCpu,"%d",nCpu);	
                    }
					// IP~1|CPUʹ����
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "1";
					strResult += cThis->m_monitorConfig.chRecordSep;
					strResult += chCpu; 
					
					cThis->m_vecPolicy[i].m_mapResult[0] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[0] = true;
					
				}

				if ( !cThis->m_vecPolicy[i].m_mapFinished[1] ) { // memory
				    MONITOR_INFO_V(string("Ready to get memory rate from ") + cThis->m_vecPolicy[i].vec_options[0]);
					int nMem;
					char chMem[4] = {0};                    
                    if ( !MonitorTask.getMemUsage(nMem)) {
						MONITOR_ERROR_S(Get MEMORY rate failed!);
                    } else {
       					sprintf(chMem,"%d",nMem);	
                    }
					// IP~2|�ڴ�ʹ����
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "2";
					strResult += cThis->m_monitorConfig.chRecordSep;
					strResult += chMem; 
					
					cThis->m_vecPolicy[i].m_mapResult[1] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[1] = true;
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[2] ) { // disk
				    MONITOR_INFO_V(string("Ready to get disk rate from ") + cThis->m_vecPolicy[i].vec_options[0]);
                    map <string,string> mapDiskUsage;
					if ( !MonitorTask.getDiskUsage(mapDiskUsage)) {
						MONITOR_ERROR_S(Get DISK rate failed!);
                    }
					
					// IP~3|����1����~����1ʹ����|...
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "3";
					strResult += cThis->m_monitorConfig.chRecordSep;
					
                    map <string,string>::iterator iter_map = mapDiskUsage.begin();
					for(; iter_map != mapDiskUsage.end(); ) {
                        strResult += iter_map->first;
						strResult += cThis->m_monitorConfig.chFieldSep;
						strResult += iter_map->second;
						
						if(++iter_map != mapDiskUsage.end()) {
							strResult += cThis->m_monitorConfig.chRecordSep;
						}
					}
					
					cThis->m_vecPolicy[i].m_mapResult[2] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[2] = true;					
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[3] ) { // service state
				    MONITOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
					map <string,string>  mapServiceState;
					if ( !MonitorTask.getServiceState(mapServiceState)) {
						MONITOR_ERROR_S(Get Service state failed!);
                    }
					
					// IP~4|service����~״̬|...
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "4";
					strResult += cThis->m_monitorConfig.chRecordSep;
					
                    map <string,string>::iterator iter_map = mapServiceState.begin();
					for(; iter_map != mapServiceState.end(); ) {
                        strResult += iter_map->first;
						strResult += cThis->m_monitorConfig.chFieldSep;
						strResult += iter_map->second;

					    if(++iter_map != mapServiceState.end()) {
						    strResult += cThis->m_monitorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[3] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[3] = true;					
				}
                //������������Ϣ
                if ( !cThis->m_vecPolicy[i].m_mapFinished[3] ) { // service state
				    MONITOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
					NetWorkInfoList NetWorkInfos;
                      char tmp[30];
                      memset(tmp,0,30);
					if ( !MonitorTask.getNetworkUsage(NetWorkInfos)) {
						MONITOR_ERROR_S(Get tNetwork Usage failed!);
                    }
					
					// ��������1~����mac��ַ~��������״̬(up/down)~��������~��������|...
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "5";
					strResult += cThis->m_monitorConfig.chRecordSep;
					
                      vector<NetworkInfo>::iterator iter = NetWorkInfos.begin();
					for(; iter != NetWorkInfos.end(); ) {
                           strResult += iter->if_name;
						strResult += cThis->m_monitorConfig.chFieldSep;
						strResult += iter->if_mac;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           itoa(iter->if_status,tmp,10);
						strResult += tmp;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           itoa(iter->if_inOctet,tmp,10);
						strResult += tmp;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           itoa(iter->if_outOctet,tmp,10);
						strResult += tmp;
					    if(++iter != NetWorkInfos.end()) {
						    strResult += cThis->m_monitorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[3] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[3] = true;					
				}
                //��װ�б���Ϣ
                if ( !cThis->m_vecPolicy[i].m_mapFinished[3] ) { // service state
				    MONITOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
					SWInstallInfoList  SWInstallInfos;
					if ( !MonitorTask.getSWInstalledUsage(SWInstallInfos)) {
						MONITOR_ERROR_S(Get SWInstalled Usage failed!);
                    }
					
					//��װ��������1~����(unknown/operatingSystem/deviceDriver/application)~������Ϣ|...
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "6";
					strResult += cThis->m_monitorConfig.chRecordSep;
					
                      vector<SWInstallInfo>::iterator iter = SWInstallInfos.begin();
					for(; iter != SWInstallInfos.end(); ) {
                           strResult += iter->sw_name;
						strResult += cThis->m_monitorConfig.chFieldSep;
                           string type = "";
                           switch(iter->sw_type)
                           {
                               case 1:
                                    type = "unknown";
                                    break;
                               case 2:
                                    type = "operatingSystem";
                                    break;
                               case 3:
                                    type = "deviceDriver";
                                    break;
                               case 4:
                                    type = "application";
                                    break;
                               default:
                                    break;
                           }
						strResult += type;
                          strResult += cThis->m_monitorConfig.chFieldSep;
                          strResult += iter->sw_date;
					    if(++iter != SWInstallInfos.end()) {
						    strResult += cThis->m_monitorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[3] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[3] = true;					
				}
			}
			sleep(10); 
		}
	}
	--cThis->m_pthreadnum_alive;
 	MONITOR_INFO_S(OnMonitorHandle Thread exit...);
	
    return NULL;
}

//base64����
string CMonitorMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}




#include "WMIMgr.h"
#include "utils/bdstring.h"

using namespace Poco;
using namespace std;


CWMIMgr::CWMIMgr(const string &strConfigName)
{
    
	m_strConfigName = strConfigName;
    memset(&m_wmiConfig,0,sizeof(m_wmiConfig));
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;

	m_wmiPtr    = NULL;
	m_listFetch.clear();
}

CWMIMgr::~CWMIMgr(void)
{
    if( m_wmiPtr != NULL ) {
        delete m_wmiPtr;
		m_wmiPtr = NULL;
	}
}

//��ʼ�����ݣ�����Start������
bool CWMIMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_wmiConfig.chLog4File) > 0 ) {
	    if(access(m_wmiConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
			 try { 
		        log4cxx::PropertyConfigurator::configure(m_wmiConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			return false;
	        }
		}
    }
	else {
        return false;
	}

	try {
		   m_wmiPtr = new CWMI();
		   //wmi initilize
		   m_wmiPtr->WmiInit(string(m_wmiConfig.chCustIP),
		   	                string(m_wmiConfig.chUserName),
		   	                string(m_wmiConfig.chPasswd));
	} catch (exception & exc){
        WMI_ERROR_S( get object CWMI failed...);
		return false;
	}
	
    WMI_INFO_S( CWMIMgr::Init() successed...);
	printConfig();

    return true;
	
}

bool CWMIMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CWMIMgr::GetModuleId(void) 
{
	return m_wmiConfig.nModuleId;
}
UInt16 CWMIMgr::GetModuleVersion(void) 
{
	return m_wmiConfig.wModuleVersion;
}
string CWMIMgr::GetModuleName(void) 
{
	return m_wmiConfig.chModuleName;
}

//��ʼ���·�)����
bool CWMIMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	/*list<modintf_datatype_t>::iterator itor_findTmp;
	itor_findTmp = find(m_listFetch.begin(),m_listFetch.end(),pDataType->nMainCategory);
	if( itor_findTmp != m_listFetch.end()) { //������ͬ����ֱ�ӷ���	
		return true;		
	}*/
	
	m_mutex_fetch.lock();
		m_listFetch.push_back(*pDataType);	//���ر���Fetch ������Ϣ
	m_mutex_fetch.unlock();
	
	return true;
}

//ֹͣ��ȡ��������
bool CWMIMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    if(m_bIsRunning) {	
        if(m_listFetch.size() > 0) {				
			m_mutex_fetch.lock();		
			list<modintf_datatype_t>::iterator itor_findTmp = m_listFetch.begin();
			do {
                if( itor_findTmp->nMainCategory == pDataType->nMainCategory ) {
	                m_listFetch.erase(itor_findTmp);  //ɾ������
	                break;
                }
				itor_findTmp++;
            }while(itor_findTmp != m_listFetch.end());
	        m_mutex_fetch.unlock();
        }
	}
    return true;	
}

bool CWMIMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "WMI LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "WMI LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_wmiConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_wmiConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_wmiConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_wmiConfig.wModuleVersion);
		GET_CONF_ITEM_CHAR(custip,m_wmiConfig.chCustIP,50);
		GET_CONF_ITEM_CHAR(username,m_wmiConfig.chUserName,50);
		GET_CONF_ITEM_CHAR(passwd,m_wmiConfig.chPasswd,50);
	}
	catch (NotFoundException& exc ) {
		cerr << "WMI LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "WMI LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

    return true;
}

void CWMIMgr::printConfig() {
	WMI_INFO_S(<============wmi configure information============>);
	WMI_INFO_V(" log4configfile=>  ["+string(m_wmiConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_wmiConfig.nModuleId);
    WMI_INFO_V(" moduleid=>        ["+string(chModuleId)+string("]"));
	WMI_INFO_V(" modulename=>      ["+string(m_wmiConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_wmiConfig.wModuleVersion);
	WMI_INFO_V(" wModuleVersion=>  ["+string(chVersion)+string("]"));
	WMI_INFO_V(" custip=>          ["+string(m_wmiConfig.chCustIP)+string("]"));
	WMI_INFO_V(" username=>        ["+string(m_wmiConfig.chUserName)+string("]"));
	WMI_INFO_V(" passwd=>          ["+string(m_wmiConfig.chPasswd)+string("]"));

	WMI_INFO_S(<============wmi configure information============>);
}

bool CWMIMgr::Load(void)
{
	return true;
}

//���÷��·�����
bool CWMIMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
	return true;
}

//���÷���ȡ����
void * CWMIMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//���÷��ͷŻ�ȡ�����ڴ�
void CWMIMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CWMIMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CWMIMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CWMIMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    return true;;
}
//Fetch Data
const char* CWMIMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    string strResult;
    switch(pDataType->nMainCategory) {
		case WMI_OPERATINGSYSTEM:		//����ϵͳ��Ϣ
		    GetOperatingSystem(strResult); break;
		case WMI_PROCESSOR:			//��������Ϣ
		    GetProcessor(strResult); break;
		case WMI_COMPUTERSYSTEM: 	//�����ڴ���Ϣ
		    GetComputerSystem(strResult); break;
		case WMI_1394CONTROLLER: 	//1394��������Ϣ
		    Get1394Controller(strResult); break;
		case WMI_BIOS:				//Bios��Ϣ
		    GetBios(strResult); break;
		case WMI_VIDEOCONTROLLER:	//�Կ���Ϣ
		    GetVideoController(strResult); break;
		case WMI_BUS:				//������Ϣ
		    GetBus(strResult); break;
		case WMI_SYSTEMSLOT: 		//ϵͳ��λ��Ϣ
		    GetSystemSlot(strResult); break;
		case WMI_PARALLELPORT:		//�˿���Ϣ
		    GetParallelPort(strResult); break;
		case WMI_PROCESS:			//������Ϣ
		    GetProcess(strResult); break;
		case WMI_DISKDRIVER: 		//Ӳ����Ϣ
		    GetDiskDriver(strResult); break;
		case WMI_NETADAPTER: 		//������������Ϣ
		    GetNetAdapter(strResult); break;
		case WMI_NETADAPTERCONFIG:	//����������������Ϣ
		    GetNetAdapterConfig(strResult); break;
		case WMI_SERVICE:			//������Ϣ
		    GetService(strResult); break;
		case WMI_PRODUCT:			//�����Ϣ
		    GetProduct(strResult); break;
		case WMI_PERFLOGICALDISK:	//ʵʱӲ��������Ϣ
		    GetPerfLogicalDisk(strResult); break;
		case WMI_PERFMEMORY: 		//ʵʱ�ڴ�������Ϣ
		    GetPerfMemory(strResult); break;
		case WMI_PERFPROCESSOR:		//ʵʱCPU������Ϣ
		    GetPerfProcessor(strResult); break;
		default: break;

    }	
	return strResult.c_str();;
}


void CWMIMgr::GetOperatingSystem(string & strResult)
{
    CWmiOperatingSystem Ctest;
    if (m_wmiPtr->WmiOperatingSystem(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetOperatingSystem failed!);
    }
}  
void CWMIMgr::GetProcessor(string & strResult)
{
    CWmiProcessor Ctest;
    if (m_wmiPtr->WmiProcessor(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetProcessor failed!);
    }
}       
void CWMIMgr::GetComputerSystem(string & strResult)
{
    CWmiComputerSystem Ctest;
    if (m_wmiPtr->WmiComputerSystem(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetComputerSystem failed!);
    }
}  
void CWMIMgr::Get1394Controller(string & strResult)
{
    CWmi1394Controller Ctest;
    if (m_wmiPtr->Wmi1394Controller(Ctest,strResult) != 0) {
		WMI_ERROR_S(Get1394Controller failed!);
    }
}  
void CWMIMgr::GetBios(string & strResult)
{
    CWmiBios Ctest;
    if (m_wmiPtr->WmiBios(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetBios failed!);
    }
}             
void CWMIMgr::GetVideoController(string & strResult)
{
    CWmiVideoController Ctest;
    if (m_wmiPtr->WmiVideoController(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetVideoController failed!);
    }
} 
void CWMIMgr::GetBus(string & strResult)
{
    //CWmiBus Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiBus(listTest,strResult) != 0) {
		WMI_ERROR_S(GetBus failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiBus*)*iter_temp;
}             
void CWMIMgr::GetSystemSlot(string & strResult)
{
    //CWmiSystemSlot Ctest;    
    Objlist listTest;
    if (m_wmiPtr->WmiSystemSlot(listTest,strResult) != 0) {
		WMI_ERROR_S(GetSystemSlot failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiSystemSlot*)*iter_temp;
}      
void CWMIMgr::GetParallelPort(string & strResult)
{
    //CWmiParallelPort Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiParallelPort(listTest,strResult) != 0) {
		WMI_ERROR_S(GetParallelPort failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWMIMgr*)*iter_temp;

}    
void CWMIMgr::GetProcess(string & strResult)
{
    //CWmiProcess Ctest; 
    Objlist listTest;
    if (m_wmiPtr->WmiProcess(listTest,strResult) != 0) {
		WMI_ERROR_S(GetProcess failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiProcess*)*iter_temp;

}        
void CWMIMgr::GetDiskDriver(string & strResult)
{
    CWmiDiskDrive Ctest;
    if (m_wmiPtr->WmiDiskDrive(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetDiskDriver failed!);
    }
}      
void CWMIMgr::GetNetAdapter(string & strResult)
{
    //CWmiNetAdpater Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiNetAdpater(listTest,strResult) != 0) {
		WMI_ERROR_S(GetNetAdapter failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiNetAdpater*)*iter_temp;

}       
void CWMIMgr::GetNetAdapterConfig(string & strResult)
{
    //CWmiNetAdpaterConfig Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiNetAdpaterConfig(listTest,strResult) != 0) {
		WMI_ERROR_S(GetNetAdapterConfig failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiNetAdpaterConfig*)*iter_temp;

}   
void CWMIMgr::GetService(string & strResult)
{
    //CWmiService Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiService(listTest,strResult) != 0) {
		WMI_ERROR_S(GetService failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiService*)*iter_temp;

}            
void CWMIMgr::GetProduct(string & strResult)
{
    //CWmiProduct Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiProduct(listTest,strResult) != 0) {
		WMI_ERROR_S(GetProduct failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiProduct*)*iter_temp;


}          
void CWMIMgr::GetPerfLogicalDisk(string & strResult)
{
    //CWmiPerfLogicalDisk Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiPerfLogicalDisk(listTest,strResult) != 0) {
		WMI_ERROR_S(GetPerfLogicalDisk failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiPerfLogicalDisk*)*iter_temp;

}   
void CWMIMgr::GetPerfMemory(string & strResult)
{
    CWmiPerfMemory Ctest;
    if (m_wmiPtr->WmiPerfMemory(Ctest,strResult) != 0) {
		WMI_ERROR_S(GetPerfMemory failed!);
    }
}         
void CWMIMgr::GetPerfProcessor(string & strResult)
{
    //CWmiPerfProcessor Ctest;
    Objlist listTest;
    if (m_wmiPtr->WmiPerfProcessor(listTest,strResult) != 0) {
		WMI_ERROR_S(GetPerfProcessor failed!);
    }
	list<long>::iterator iter_temp = listTest.begin();
	for(;iter_temp != listTest.end();iter_temp++)
		delete (CWmiPerfProcessor*)*iter_temp;

}     


//����ģ��
bool CWMIMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	if (! m_bIsRunning ) {
        pthread_attr_t pa;       //�߳�����
        pthread_attr_init(&pa);	//��ʼ���߳�����
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //�����߳�����
        if (pthread_create(&p_thread_fetch, &pa, OnFetchHandle, this) != 0) {
			WMI_ERROR_S(Create OnFetchHandle Thread failed!);
			return false;
        }
		WMI_INFO_S(Create OnFetchHandle Thread successed!);
		m_bIsRunning = true;
	}
	WMI_INFO_S(CWMIMgr::Start successed!);
	return m_bIsRunning;
}

//ֹͣģ��
bool CWMIMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
    m_bIsRunning = false;
	WMI_INFO_S(CWMIMgr::Stop successed!);
    return true;
}

void *CWMIMgr::OnFetchHandle(void *arg) {
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CWMIMgr *WMIThis = reinterpret_cast<CWMIMgr*>(arg);
	//pthread_cleanup_push(WMIThis->OnFetchCleanUp,arg);
    while(WMIThis->m_bIsRunning) {	
        if(WMIThis->m_listFetch.size() > 0) {
			list<modintf_datatype_t>::iterator itor_Tmp = WMIThis->m_listFetch.begin();
	        UInt32 dwRetLen = 0;
	        string strResult = WMIThis->FetchData(&(*itor_Tmp), dwRetLen);
			//WMIThis->m_pFuncFetchData(WMIThis->m_wmiConfig.nModuleId, &(*itor_Tmp),dwRetLen);
			
	        if(! WMIThis->m_pFuncReportData(WMIThis->m_wmiConfig.nModuleId, 
				                            &(*itor_Tmp),
				                            (void *)&strResult,dwRetLen) ) {
                WMI_INFO_S(Report data failed!);
	        }
	        else {
			    WMI_INFO_S(Report data success!);
	        }

		    WMIThis->m_mutex_fetch.lock();
	        WMIThis->m_listFetch.pop_front();  //ɾ���Ѿ���ɵ�Fetch ����
	        WMIThis->m_mutex_fetch.unlock();
        }
	}
		//pthread_testcancel(); //�ӳ�ȡ����
		WMIThis->m_listFetch.clear(); //stopģ����������δ�������
		return NULL;
}

//��ȡ��־
void *CWMIMgr::OnReportHandle(void *arg)
{
    return NULL;
}

//base64����
string CWMIMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



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

//初始化数据，先于Start被调用
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

//开始（下发)任务
bool CWMIMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	/*list<modintf_datatype_t>::iterator itor_findTmp;
	itor_findTmp = find(m_listFetch.begin(),m_listFetch.end(),pDataType->nMainCategory);
	if( itor_findTmp != m_listFetch.end()) { //已有相同任务直接返回	
		return true;		
	}*/
	
	m_mutex_fetch.lock();
		m_listFetch.push_back(*pDataType);	//本地保存Fetch 任务信息
	m_mutex_fetch.unlock();
	
	return true;
}

//停止（取消）任务
bool CWMIMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    if(m_bIsRunning) {	
        if(m_listFetch.size() > 0) {				
			m_mutex_fetch.lock();		
			list<modintf_datatype_t>::iterator itor_findTmp = m_listFetch.begin();
			do {
                if( itor_findTmp->nMainCategory == pDataType->nMainCategory ) {
	                m_listFetch.erase(itor_findTmp);  //删除任务
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

//调用方下发数据
bool CWMIMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
	return true;
}

//调用方获取数据
void * CWMIMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
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
		case WMI_OPERATINGSYSTEM:		//操作系统信息
		    GetOperatingSystem(strResult); break;
		case WMI_PROCESSOR:			//处理器信息
		    GetProcessor(strResult); break;
		case WMI_COMPUTERSYSTEM: 	//计算内存信息
		    GetComputerSystem(strResult); break;
		case WMI_1394CONTROLLER: 	//1394控制器信息
		    Get1394Controller(strResult); break;
		case WMI_BIOS:				//Bios信息
		    GetBios(strResult); break;
		case WMI_VIDEOCONTROLLER:	//显卡信息
		    GetVideoController(strResult); break;
		case WMI_BUS:				//总线信息
		    GetBus(strResult); break;
		case WMI_SYSTEMSLOT: 		//系统槽位信息
		    GetSystemSlot(strResult); break;
		case WMI_PARALLELPORT:		//端口信息
		    GetParallelPort(strResult); break;
		case WMI_PROCESS:			//进程信息
		    GetProcess(strResult); break;
		case WMI_DISKDRIVER: 		//硬盘信息
		    GetDiskDriver(strResult); break;
		case WMI_NETADAPTER: 		//网络适配器信息
		    GetNetAdapter(strResult); break;
		case WMI_NETADAPTERCONFIG:	//网络适配器配置信息
		    GetNetAdapterConfig(strResult); break;
		case WMI_SERVICE:			//服务信息
		    GetService(strResult); break;
		case WMI_PRODUCT:			//软件信息
		    GetProduct(strResult); break;
		case WMI_PERFLOGICALDISK:	//实时硬盘性能信息
		    GetPerfLogicalDisk(strResult); break;
		case WMI_PERFMEMORY: 		//实时内存性能信息
		    GetPerfMemory(strResult); break;
		case WMI_PERFPROCESSOR:		//实时CPU性能信息
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


//启动模块
bool CWMIMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
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

//停止模块
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
	        WMIThis->m_listFetch.pop_front();  //删除已经完成的Fetch 任务
	        WMIThis->m_mutex_fetch.unlock();
        }
	}
		//pthread_testcancel(); //延迟取消点
		WMIThis->m_listFetch.clear(); //stop模块就清除所有未完成任务
		return NULL;
}

//获取日志
void *CWMIMgr::OnReportHandle(void *arg)
{
    return NULL;
}

//base64解码
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


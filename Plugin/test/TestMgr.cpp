
#include "TestMgr.h"
#include "utils/bdstring.h"
#include <iostream>

using namespace Poco;
using namespace std;

CTestMgr::CTestMgr(const string &strConfigName)
{
	m_strConfigName = strConfigName;
    memset(&m_testConfig,0,sizeof(m_testConfig));
	
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;
	pServer           = NULL;

	m_vecPolicy.clear();
}

CTestMgr::~CTestMgr(void)
{
	m_vecPolicy.clear();
}

//初始化数据，先于Start被调用
bool CTestMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}
    if( strlen(m_testConfig.chLog4File) > 0 ) {
	    if(access(m_testConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
			 try { 
		        log4cxx::PropertyConfigurator::configure(m_testConfig.chLog4File);
	         } catch (Exception &) { 
			     return false;
	         }
		}
    }
	else {
        return false;
	}
	
    TEST_INFO_S( CTestMgr::Init() successed...);
	printConfig();

    //Start();
    return true;
	
}

//检查模块是否处于运行状态
bool CTestMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}

UInt32 CTestMgr::GetModuleId(void) 
{
	return m_testConfig.nModuleId;
}

UInt16 CTestMgr::GetModuleVersion(void) 
{
	return m_testConfig.wModuleVersion;
}

string CTestMgr::GetModuleName(void) 
{
	return m_testConfig.chModuleName;
}

//开始（下发)任务
bool CTestMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	return true;
}

//停止（取消）任务
bool CTestMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CTestMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "SysLog LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "SysLog LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_testConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_testConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_testConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_testConfig.wModuleVersion);		 
		GET_CONF_ITEM_INT(sysloglistenport,m_testConfig.nListenPort);
		GET_CONF_ITEM_CHAR(record_separator,m_testConfig.chRecordSep,2);
		GET_CONF_ITEM_CHAR(field_separator,m_testConfig.chFieldSep,2);

	}
	catch (NotFoundException& exc ) {
		cerr << "SysLog LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "SysLog LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

    return true;
}

void CTestMgr::printConfig() {
	TEST_INFO_S(<=========test configure information=========>);
	TEST_INFO_V(" log4configfile=>   ["+string(m_testConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_testConfig.nModuleId);
    TEST_INFO_V(" moduleid=>         ["+string(chModuleId)+string("]"));
	TEST_INFO_V(" modulename=>       ["+string(m_testConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_testConfig.wModuleVersion);
	TEST_INFO_V(" wModuleVersion=>   ["+string(chVersion)+string("]"));
	char chPort[6] = {0};
	sprintf(chPort,"%d",m_testConfig.nListenPort);
	TEST_INFO_V(" sysloglistenport=> ["+string(chPort)+string("]"));
	TEST_INFO_V(" record_separator=> ["+string(m_testConfig.chRecordSep)+string("]"));
	TEST_INFO_V(" field_separator=>  ["+string(m_testConfig.chFieldSep)+string("]"));
	TEST_INFO_S(<=========test configure information=========>);
}

bool CTestMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CTestMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
	string str = m_testConfig.chFieldSep;
	string_split(strPolicy,str,m_vecPolicy);
	vector<string>::iterator iter = m_vecPolicy.begin();
	TEST_INFO_S(<=========TEST POlOCY=========>);
	for(;iter!= m_vecPolicy.end();iter++)
		TEST_INFO_V(" TEST POlOCY =>" + *iter);
	TEST_INFO_S(<=========TEST POlOCY=========>);
	return true;
}

//调用方获取数据
void * CTestMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CTestMgr::FreeData(void * pData)
{
	
}

//set callback for report function
void CTestMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}

//set call back for fetch function
void CTestMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CTestMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    bool bRet = false;
	
    if (m_pFuncReportData){
        bRet =  m_pFuncReportData(m_testConfig.nModuleId,pDataType,pData,dwLength);
    }
    return bRet;
}

//Fetch Data
const char* CTestMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CTestMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	//stop后会导致单例失效
    pServer = TestUdpClientService::inistance();
    if( pServer == NULL ) return false;
    if( !m_bIsRunning) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        if (pthread_create(&p_thread_udp, &pa, OnSatrtUdpServer, this) != 0) {
			TEST_ERROR_S(Create OnSatrtUdpServer Thread failed!);
			return false;
        }
		TEST_INFO_S(Create OnSatrtUdpServer Thread successed!);

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			TEST_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		TEST_INFO_S(Create OnReportHandle Thread successed!);	
	}
	usleep(20000);// 20毫秒，启动线程需要一些执行时间
	TEST_INFO_S(CTestMgr::Start successed!);
	
	return true;
}

//停止模块
bool CTestMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
    //停UDP线程,状态由线程自己改变
    pServer->handle_close(pServer->get_handle(), 0);
	// m_bIsRunning状态在OnSatrtUdpServer中改变
	//ACE_Reactor::instance()->end_event_loop();	
        TestUdpClientService::pReactor->end_reactor_event_loop();

	TEST_INFO_S(CTestMgr::Stop finished!);	
    return true;
}

void *CTestMgr::OnFetchHandle(void *arg) {
    return NULL;
}

//开启UDP监听以及I/O事件检测
void* CTestMgr::OnSatrtUdpServer(void *arg)
{
	CTestMgr *cThis = reinterpret_cast<CTestMgr*>(arg);

	//ACE_Reactor* pReactor = ACE_Reactor::instance();
	//cThis->pServer->reactor(pReactor);
       TestUdpClientService::pReactor = new ACE_Reactor( (ACE_Reactor_Impl *)new ACE_TP_Reactor(), true);
	if( cThis->pServer->open(cThis->m_testConfig.nListenPort) != 0 ) {
		cThis->pServer->handle_close(ACE_INVALID_HANDLE, 0);	
		cThis->m_bIsRunning = false;
		TEST_ERROR_S(OnSatrtUdpServer end ...);
		return NULL;
	}
	cThis->pServer->pReactor->run_reactor_event_loop(); //阻塞，内部循环
	TEST_INFO_S(run_reactor_event_loop is ended !);	

	cThis->pServer->pReactor->close();

	if ( cThis->pServer->pReactor != NULL )
		delete cThis->pServer->pReactor;
	cThis->pServer->pReactor = NULL;

	cThis->m_bIsRunning = false;
	TEST_INFO_S(OnSatrtUdpServer end ...);
	return NULL;
}

//获取日志
void *CTestMgr::OnReportHandle(void *arg)
{   
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CTestMgr *cThis = reinterpret_cast<CTestMgr*>(arg);
    cThis->m_bIsRunning = true;
	while( cThis->m_bIsRunning ) {
		if ( cThis->pServer->m_strLogList.size() > 0) {	
			list<string>::iterator iter = cThis->pServer->m_strLogList.begin();
            cThis->pServer->m_inputMutex.lock();
			for(;iter != cThis->pServer->m_strLogList.end();) {
				// 过滤策略中指定 IP 设备日志
				string::size_type firstpos = (*iter).find(cThis->m_testConfig.chFieldSep);
				string strSrcIP = (*iter).substr(firstpos+1,(*iter).find(cThis->m_testConfig.chFieldSep,firstpos+1)-firstpos-1);
				TEST_DEBUG_V("Source IP :"+ strSrcIP);
				if(find(cThis->m_vecPolicy.begin(),cThis->m_vecPolicy.end(),strSrcIP) == cThis->m_vecPolicy.end()) {
					modintf_datatype_t temp;
					temp.nDataType     = MODULE_DATA_LOG;
					temp.nMainCategory = MODULE_DATA_LOG_SYSLOG;
					//cThis->ReportData(&temp,(void *)&(*iter),iter->length());
				}	
				iter = cThis->pServer->m_strLogList.erase(iter);
			}
			cThis->pServer->m_inputMutex.unlock();					
		}   
	}
	TEST_INFO_S(OnReportHandle end ...);
    return NULL;
}

//base64解码
string CTestMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}


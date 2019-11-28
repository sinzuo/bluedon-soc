
#include "SnmpTrapMgr.h"
#include "utils/bdstring.h"
#include <iostream>

using namespace Poco;
using namespace std;

CSnmpTrapMgr::CSnmpTrapMgr(const string &strConfigName)
{
	m_strConfigName = strConfigName;
    memset(&m_snmpTrapConfig,0,sizeof(m_snmpTrapConfig));
	
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;
    m_bUdpServerState = false;
	pServer           = NULL;

	m_listPolicy.clear();
}

CSnmpTrapMgr::~CSnmpTrapMgr(void)
{
	m_listPolicy.clear();
}

//初始化数据，先于Start被调用
bool CSnmpTrapMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_snmpTrapConfig.chLog4File) > 0 ) {
	    if(access(m_snmpTrapConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
			 try { 
		        log4cxx::PropertyConfigurator::configure(m_snmpTrapConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	         } catch (Exception &) { 
			     return false;
	         }
		}
    }
	else {
        return false;
	}
	
    SNMPTRAP_INFO_S( CSnmpTrapMgr::Init() successed...);
	printConfig();

    return true;
	
}

//检查模块是否处于运行状态
bool CSnmpTrapMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}

UInt32 CSnmpTrapMgr::GetModuleId(void) 
{
	return m_snmpTrapConfig.nModuleId;
}

UInt16 CSnmpTrapMgr::GetModuleVersion(void) 
{
	return m_snmpTrapConfig.wModuleVersion;
}

string CSnmpTrapMgr::GetModuleName(void) 
{
	return m_snmpTrapConfig.chModuleName;
}

//开始（下发)任务
bool CSnmpTrapMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	return true;
}

//停止（取消）任务
bool CSnmpTrapMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CSnmpTrapMgr::LoadConfig(void)
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
		GET_CONF_ITEM_CHAR(log4configfile,m_snmpTrapConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_snmpTrapConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_snmpTrapConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_snmpTrapConfig.wModuleVersion);		 
		GET_CONF_ITEM_CHAR(listenportstr,m_snmpTrapConfig.chListenPort,20);
		GET_CONF_ITEM_CHAR(record_separator,m_snmpTrapConfig.chRecordSep,2);
		GET_CONF_ITEM_CHAR(field_separator,m_snmpTrapConfig.chFieldSep,2);
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

void CSnmpTrapMgr::printConfig() {
	SNMPTRAP_INFO_S(<========snmptrap configure information========>);
	SNMPTRAP_INFO_V(" log4configfile=>  ["+string(m_snmpTrapConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_snmpTrapConfig.nModuleId);
    SNMPTRAP_INFO_V(" moduleid=>        ["+string(chModuleId)+string("]"));
	SNMPTRAP_INFO_V(" modulename=>      ["+string(m_snmpTrapConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_snmpTrapConfig.wModuleVersion);
	SNMPTRAP_INFO_V(" ModuleVersion=>   ["+string(chVersion)+string("]"));
	SNMPTRAP_INFO_V(" listenportstr=>   ["+string(m_snmpTrapConfig.chListenPort)+string("]"));
	SNMPTRAP_INFO_S(<========snmptrap configure information========>);
}

bool CSnmpTrapMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CSnmpTrapMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_listPolicy.clear();
	string str = m_snmpTrapConfig.chFieldSep;
	string_split(strPolicy,str,m_listPolicy);
	vector<string>::iterator iter = m_listPolicy.begin();
	SNMPTRAP_INFO_S(<========SNMPTRAP POlOCY========>);
	for(;iter!= m_listPolicy.end();iter++)
		SNMPTRAP_INFO_V(" SNMPTRAP POlOCY =>" + *iter);
	SNMPTRAP_INFO_S(<========SNMPTRAP POlOCY========>);
	return true;
}

//调用方获取数据
void * CSnmpTrapMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CSnmpTrapMgr::FreeData(void * pData)
{
	
}

//set callback for report function
void CSnmpTrapMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}

//set call back for fetch function
void CSnmpTrapMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CSnmpTrapMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    if (m_pFuncReportData){
        return m_pFuncReportData(m_snmpTrapConfig.nModuleId,pDataType,pData,dwLength);
    }
    return false;
}

//Fetch Data
const char* CSnmpTrapMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CSnmpTrapMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);

	//stop后会导致单例失效
    pServer = CSnmpTrapServer::inistance();
    if( pServer == NULL ) return false;

    if ( !m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        if (pthread_create(&p_thread_snmptrap, &pa, OnSnmpTrapServer, this) != 0) {
			SNMPTRAP_ERROR_S(Create OnSnmpTrapServer Thread failed!);
			return false;
        }
		SNMPTRAP_INFO_S(Create OnSnmpTrapServer Thread successed!);

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			SNMPTRAP_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		SNMPTRAP_INFO_S(Create OnReportHandle Thread successed!);
	}	
	usleep(20000);   //20毫秒，启动线程需要一些执行时间
	SNMPTRAP_INFO_S(CSnmpTrapMgr::Start successed!);
	
	return true;
}

//停止模块
bool CSnmpTrapMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);

	//由OnSnmpTrapServer修改m_bIsRunning状态
	pServer->setRunningFlag(false);
	
	SNMPTRAP_INFO_S(CSnmpTrapMgr::Stop successed!);
	
    return true;
}

void *CSnmpTrapMgr::OnFetchHandle(void *arg) {
    return NULL;
}

//开启UDP监听以及I/O事件检测
void* CSnmpTrapMgr::OnSnmpTrapServer(void *arg)
{
	CSnmpTrapMgr *cThis = reinterpret_cast<CSnmpTrapMgr*>(arg);
    if(cThis->pServer->Initlize() != 0) {
		SNMPTRAP_ERROR_S(CSnmpTrapServer::Initlize failed!);
        return NULL;
    }
	string strListenPorts = cThis->m_snmpTrapConfig.chListenPort;
	if(strListenPorts.length() != 0) {
		cThis->pServer->SetLinstenPort(strListenPorts);
	}
	SNMPTRAP_INFO_V("ListenPort string => " + strListenPorts);
	if(cThis->pServer->BindAll() != 0) {
        SNMPTRAP_ERROR_S(CSnmpTrapServer::BindAll failed!);
        return NULL;
	}

    cThis->m_bUdpServerState = true;

	SNMPTRAP_INFO_S(snmptrapd_main_loop start...);
	cThis->pServer->snmptrapd_main_loop(); //阻塞，内部循环
	SNMPTRAP_INFO_S(snmptrapd_main_loop end...);

    //cThis->pServer->handle_close();
    cThis->m_bUdpServerState = false;
	SNMPTRAP_INFO_S(OnSnmpTrapServer end ...);
	return NULL;
}

//获取日志
void *CSnmpTrapMgr::OnReportHandle(void *arg)
{   
    //if (pthread_detach(pthread_self()) ==0 ) SNMPTRAP_DEBUG_S(start thread with detach state...);
	CSnmpTrapMgr *cThis = reinterpret_cast<CSnmpTrapMgr*>(arg);
    while(true) {
        if (cThis->m_bUdpServerState) {  // udp 线程启动后，插件状态才更新为true
            cThis->m_bIsRunning = true;
        }
//        string strIsRunning = cThis->m_bIsRunning? "1":"0";
//        SNMPTRAP_DEBUG_V("strIsRunning = " + strIsRunning);
        while( cThis->m_bUdpServerState && cThis->m_bIsRunning ) {
            if(cThis->pServer->getLogList().size() == 0)
            {
                sleep(1);
                continue;
            }

            list<string>::iterator iter = cThis->pServer->getLogList().begin();
            if ( iter != cThis->pServer->getLogList().end()) {
                   cThis->pServer->lockMutex();
                   string s_iter = *iter;
                  // 过滤策略中指定 IP 设备日志
                  string::size_type firstpos = (*iter).find(cThis->m_snmpTrapConfig.chFieldSep);
                  string strSrcIP = (*iter).substr(firstpos+1,(*iter).find(cThis->m_snmpTrapConfig.chFieldSep,firstpos+1)-firstpos-1);
                  if(find(cThis->m_listPolicy.begin(),cThis->m_listPolicy.end(),strSrcIP) == cThis->m_listPolicy.end()) {
                      SNMPTRAP_DEBUG_V("Source IP :"+ strSrcIP);
                      modintf_datatype_t temp;
                      temp.nDataType     = MODULE_DATA_LOG;
                      temp.nMainCategory = MODULE_DATA_LOG_SNMPTRAP;
                      cThis->ReportData(&temp,(void *)&(s_iter),s_iter.length());
                      cThis->pServer->LogList_erase(s_iter);
                      cThis->pServer->unlockMutex();
                  }
            }

            usleep(10 * 1000);
        }

        if ( !cThis->m_bUdpServerState && cThis->m_bIsRunning) { // dup 线程退出了
            break;
        }
        sleep(1);
    }
    cThis->pServer->handle_close();
    cThis->pServer = NULL;
    cThis->m_bIsRunning = false;
    SNMPTRAP_INFO_S(OnReportHandle exit ...);

    return NULL;
}

//base64解码
string CSnmpTrapMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}


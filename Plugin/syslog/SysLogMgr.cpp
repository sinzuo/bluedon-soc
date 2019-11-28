
#include "SysLogMgr.h"
#include "utils/bdstring.h"
#include <iostream>

using namespace Poco;
using namespace std;

Poco::Mutex  ShareData::m_inputMutex;
list<string> ShareData::m_strLogList;
string ShareData::m_strDelimiter;
long ShareData::list_num = 0 ;

CSysLogMgr::CSysLogMgr(const string &strConfigName)
{
	m_strConfigName = strConfigName;
    memset(&m_sysLogConfig,0,sizeof(m_sysLogConfig));
	
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;
    m_bServerState = false;
	pServer           = NULL;
    pTcpServer        = NULL;

    m_setPolicy.clear();
}

CSysLogMgr::~CSysLogMgr(void)
{
    m_setPolicy.clear();
}

//初始化数据，先于Start被调用
bool CSysLogMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}
    if( strlen(m_sysLogConfig.chLog4File) > 0 ) {
	    if(access(m_sysLogConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
			 try { 
		        log4cxx::PropertyConfigurator::configure(m_sysLogConfig.chLog4File);
	         } catch (Exception &) { 
			     return false;
	         }
		}
    }
	else {
        return false;
	}
	
    SYSLOG_INFO_S( CSysLogMgr::Init() successed...);
	printConfig();

    return true;
	
}

//检查模块是否处于运行状态
bool CSysLogMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}

//获取模块id
UInt32 CSysLogMgr::GetModuleId(void) 
{
	return m_sysLogConfig.nModuleId;
}

//获取模块版本
UInt16 CSysLogMgr::GetModuleVersion(void) 
{
	return m_sysLogConfig.wModuleVersion;
}

//获取模块名称
string CSysLogMgr::GetModuleName(void) 
{
	return m_sysLogConfig.chModuleName;
}

//开始（下发)任务
bool CSysLogMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	return true;
}

//停止（取消）任务
bool CSysLogMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

//加载配置
bool CSysLogMgr::LoadConfig(void)
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
		GET_CONF_ITEM_CHAR(log4configfile,m_sysLogConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_sysLogConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_sysLogConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_sysLogConfig.wModuleVersion);		 
		GET_CONF_ITEM_INT(sysloglistenport,m_sysLogConfig.nListenPort);
		GET_CONF_ITEM_CHAR(record_separator,m_sysLogConfig.chRecordSep,2);
		GET_CONF_ITEM_CHAR(field_separator,m_sysLogConfig.chFieldSep,2);
        GET_CONF_ITEM_BOOL(use_udp_recv,m_sysLogConfig.bUseUdpRecv);
        GET_CONF_ITEM_INT(listcontrol,m_sysLogConfig.nListCtrl);
        GET_CONF_ITEM_INT(breakcontrol,m_sysLogConfig.nBreakCtrl);
        GET_CONF_ITEM_INT(sleeptimems,m_sysLogConfig.nSleepTimeMs);
		GET_CONF_ITEM_CHAR(delimiter,m_sysLogConfig.chDelimiter,20);
        if(strcmp(m_sysLogConfig.chDelimiter,"") == 0)
        {
            strcpy(m_sysLogConfig.chDelimiter, "\n");  //如果配置文件里面没有定义，默认为\n
        }
		GET_CONF_ITEM_BOOL(sendtokafka,m_sysLogConfig.nSendtoKafka);
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
//打印配置
void CSysLogMgr::printConfig() {
	SYSLOG_INFO_S(<=========syslog configure information=========>);
	SYSLOG_INFO_V(" log4configfile=>   ["+string(m_sysLogConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_sysLogConfig.nModuleId);
    SYSLOG_INFO_V(" moduleid=>         ["+string(chModuleId)+string("]"));
	SYSLOG_INFO_V(" modulename=>       ["+string(m_sysLogConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_sysLogConfig.wModuleVersion);
	SYSLOG_INFO_V(" wModuleVersion=>   ["+string(chVersion)+string("]"));
	char chPort[6] = {0};
	sprintf(chPort,"%d",m_sysLogConfig.nListenPort);
	SYSLOG_INFO_V(" sysloglistenport=> ["+string(chPort)+string("]"));
	SYSLOG_INFO_V(" record_separator=> ["+string(m_sysLogConfig.chRecordSep)+string("]"));
	SYSLOG_INFO_V(" field_separator=>  ["+string(m_sysLogConfig.chFieldSep)+string("]"));
	SYSLOG_INFO_S(<=========syslog configure information=========>);
}

bool CSysLogMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CSysLogMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
    m_setPolicy.clear();
	string str = m_sysLogConfig.chFieldSep;
    vector<string> vecPolicy;
    string_split(strPolicy,str,vecPolicy);
    vector<string>::iterator iter = vecPolicy.begin();
	SYSLOG_INFO_S(<=========SYSLOG POlOCY=========>);
    for(;iter!= vecPolicy.end();iter++)
    {
        SYSLOG_INFO_V(" SYSLOG POlOCY =>" + *iter);
        m_setPolicy.insert(*iter); //放入set中
    }
	SYSLOG_INFO_S(<=========SYSLOG POlOCY=========>);
	return true;
}

//调用方获取数据
void * CSysLogMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CSysLogMgr::FreeData(void * pData)
{
	
}

//set callback for report function
void CSysLogMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}

//set call back for fetch function
void CSysLogMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CSysLogMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    bool bRet = false;
	
    if (m_pFuncReportData){
        bRet =  m_pFuncReportData(m_sysLogConfig.nModuleId,pDataType,pData,dwLength);
    }
    return bRet;
}

//Fetch Data
const char* CSysLogMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CSysLogMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	//stop后会导致单例失效
    if(m_sysLogConfig.bUseUdpRecv) {
        pServer = UdpClientService::inistance();
        if( pServer == NULL ) return false;
    }
    else {
        pTcpServer = TcpClientAcceptor::inistance();
        if( pTcpServer == NULL ) return false;
    }

if(strcmp(m_sysLogConfig.chDelimiter ,"")!=0) ShareData::m_strDelimiter = m_sysLogConfig.chDelimiter;
    if( !m_bIsRunning) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        int ret = 0;

        if(m_sysLogConfig.bUseUdpRecv) {
            ret = pthread_create(&p_thread_server, &pa, OnSatrtUdpServer, this);
        }
        else {
            ret = pthread_create(&p_thread_server, &pa, OnSatrtTcpServer, this);
        }

        if (ret != 0) {
            SYSLOG_ERROR_S(Create OnSatrtxxpServer Thread failed!);
			return false;
        }
		usleep(50000);// 50毫秒
        SYSLOG_INFO_S(Create OnSatrtxxpServer Thread successed!);

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			SYSLOG_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		SYSLOG_INFO_S(Create OnReportHandle Thread successed!);	
	}
	usleep(50000);// 50毫秒
	SYSLOG_INFO_S(CSysLogMgr::Start successed!);
	
	return true;
}

//停止模块
bool CSysLogMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
    //停UDP线程,状态由线程自己改变

    if(m_sysLogConfig.bUseUdpRecv) {
        pServer->handle_close(pServer->get_handle(), 0);
    }
    else {
        pTcpServer->handle_close(pTcpServer->get_handle(), 0);
    }
	// m_bIsRunning状态在OnSatrtUdpServer中改变
	ACE_Reactor::instance()->end_event_loop();	

	SYSLOG_INFO_S(CSysLogMgr::Stop finished!);	
    return true;
}

void *CSysLogMgr::OnFetchHandle(void *arg) {
    return NULL;
}

//开启UDP监听以及I/O事件检测
void* CSysLogMgr::OnSatrtUdpServer(void *arg)
{
	CSysLogMgr *cThis = reinterpret_cast<CSysLogMgr*>(arg);
	ACE_Reactor* pReactor = ACE_Reactor::instance();
	cThis->pServer->reactor(pReactor);
	if( cThis->pServer->open(cThis->m_sysLogConfig.nListenPort) != 0 ) {
		cThis->pServer->handle_close(ACE_INVALID_HANDLE, 0);	
		SYSLOG_ERROR_S(OnSatrtUdpServer end ...);
        cThis->m_bIsRunning = true;
		return NULL;
	}
    cThis->m_bServerState = true;
	pReactor->run_reactor_event_loop(); //阻塞，内部循环

	pReactor->close_singleton();
	sleep(1);
    cThis->m_bServerState = false;
    SYSLOG_INFO_S(OnSatrtUdpServer exit ...);
	return NULL;
}

//开启TCP监听以及I/O事件检测
void* CSysLogMgr::OnSatrtTcpServer(void *arg)
{
    CSysLogMgr *cThis = reinterpret_cast<CSysLogMgr*>(arg);
    ACE_Reactor* pReactor = ACE_Reactor::instance();
    cThis->pTcpServer->reactor(pReactor);
    if( cThis->pTcpServer->open(cThis->m_sysLogConfig.nListenPort) != 0 ) {
        cThis->pTcpServer->handle_close(ACE_INVALID_HANDLE, 0);
        SYSLOG_ERROR_S(OnSatrtUdpServer end ...);
        cThis->m_bIsRunning = true;
        return NULL;
    }
    cThis->m_bServerState = true;
    pReactor->run_reactor_event_loop(); //阻塞，内部循环

    pReactor->close_singleton();
    cThis->m_bServerState = false;
    SYSLOG_INFO_S(OnSatrtUdpServer exit ...);
    return NULL;
}

//获取日志
void *CSysLogMgr::OnReportHandle(void *arg)
{   
	CSysLogMgr *cThis = reinterpret_cast<CSysLogMgr*>(arg);

	while(true) {
        if (cThis->m_bServerState) {  // udp 线程启动后，插件状态才更新为true
			cThis->m_bIsRunning = true;
		}
        //bool  bReportCtrl = false;
		long  nBreakCtrl = 0;
		string msg;
        while( cThis->m_bServerState && cThis->m_bIsRunning) {

            //static int num = 0;

            nBreakCtrl = 0;
            list<string>::iterator iter = ShareData::m_strLogList.begin();

            while(iter != ShareData::m_strLogList.end()) {
                msg = *iter;

                ShareData::m_inputMutex.lock();
                ShareData::m_strLogList.pop_front();
                iter = ShareData::m_strLogList.begin();
                ShareData::list_num--;
                ShareData::m_inputMutex.unlock();

                string::size_type firstpos = msg.find(cThis->m_sysLogConfig.chFieldSep);
                string strSrcIP = msg.substr(firstpos+1,msg.find(cThis->m_sysLogConfig.chFieldSep,firstpos+1)-firstpos-1);
                // 过滤策略中指定 IP 设备日志
                if(cThis->m_setPolicy.find(strSrcIP) == cThis->m_setPolicy.end()) {
                modintf_datatype_t temp;
                temp.nDataType	   = MODULE_DATA_LOG;
                temp.nMainCategory = MODULE_DATA_LOG_SYSLOG;
                if(cThis->m_sysLogConfig.nSendtoKafka)
                {
                    temp.dwPackFlag = 1; //SEND_DATA_TO_KAFKA
                }
                //cThis->ReportData(&temp,(void *)&(*iter),iter->length());
                //SYSLOG_DEBUG_V("Send :"+ smg);
                cThis->m_pFuncReportData(cThis->m_sysLogConfig.nModuleId,&temp,(void *)&msg,msg.length());
                ++nBreakCtrl;
                }
                if (nBreakCtrl > cThis->m_sysLogConfig.nBreakCtrl) break;
            }


            if(nBreakCtrl > cThis->m_sysLogConfig.nBreakCtrl){  //控制睡眠时间
                usleep(1000);
            } else{
                usleep(cThis->m_sysLogConfig.nSleepTimeMs * 1000);
                //SYSLOG_DEBUG_V("usleep : runs");
            }
            //  if(ShareData::m_strLogList.size() == 0)
            //      usleep(cThis->m_sysLogConfig.nSleepTimeMs * 1000);
            //  else
            //      usleep(10 * 1000);
        }

        if ( !cThis->m_bServerState && cThis->m_bIsRunning) { // dup 线程退出了
            break;
        }
		sleep(1);
	}
    if(cThis->m_sysLogConfig.bUseUdpRecv) cThis->pServer->Destroy();
    else cThis->pTcpServer->Destroy();

    ShareData::m_inputMutex.lock();
    ShareData::m_strLogList.clear();
    ShareData::m_inputMutex.unlock();

	cThis->m_bIsRunning = false;
    SYSLOG_INFO_S(OnReportHandle exit ...);
	//pthread_exit(NULL);
    return NULL;
}

//base64解码
string CSysLogMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}


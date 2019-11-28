
#include "FlowMgr.h"
#include "utils/bdstring.h"
#include <iostream>

using namespace Poco;
using namespace std;

CFlowMgr::CFlowMgr(const string &strConfigName)
{
    m_strConfigName = strConfigName;
    memset(&m_flowConfig,0,sizeof(m_flowConfig));

    m_pFuncReportData = NULL;
    m_pFuncFetchData  = NULL;
    m_bIsRunning      = false;
	m_bUdpServerState = false;
    pServer           = NULL;

    m_vecPolicy.clear();
}

CFlowMgr::~CFlowMgr(void)
{
    m_vecPolicy.clear();
}

//初始化数据，先于Start被调用
bool CFlowMgr::Init(void)
{
    if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
    }

    if(!LoadConfig()) {
        return false;
    }
    if( strlen(m_flowConfig.chLog4File) > 0 ) {
        if(access(m_flowConfig.chLog4File, F_OK) != 0) {
            return false;
        }
        else {
             try {
                log4cxx::PropertyConfigurator::configure(m_flowConfig.chLog4File);
             } catch (Exception &) {
                 return false;
             }
        }
    }
    else {
        return false;
    }

    FLOW_INFO_S( CFlowMgr::Init() successed...);
    printConfig();

    return true;

}

//检查模块是否处于运行状态
bool CFlowMgr::IsRunning(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
    return m_bIsRunning;
}

UInt32 CFlowMgr::GetModuleId(void)
{
    return m_flowConfig.nModuleId;
}

UInt16 CFlowMgr::GetModuleVersion(void)
{
    return m_flowConfig.wModuleVersion;
}

string CFlowMgr::GetModuleName(void)
{
    return m_flowConfig.chModuleName;
}

//开始（下发)任务
bool CFlowMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;
}

//停止（取消）任务
bool CFlowMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;
}

bool CFlowMgr::LoadConfig(void)
{
    AutoPtr<IniFileConfiguration> pConf = NULL;
    try {
        pConf = new IniFileConfiguration(m_strConfigName);
    } catch (NotFoundException& exc ) {
        cerr << "Flow LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;
    } catch (SyntaxException& exc) {
        cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;
    } catch (Exception& exc) {
        cerr << "Flow LoadConfig:" << exc.displayText() << endl;
        return false;
    }

    try {
        GET_CONF_ITEM_CHAR(log4configfile,m_flowConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_flowConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_flowConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_flowConfig.wModuleVersion);
        GET_CONF_ITEM_INT(sysloglistenport,m_flowConfig.nListenPort);
        GET_CONF_ITEM_CHAR(record_separator,m_flowConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_flowConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(listcontrol,m_flowConfig.nListCtrl);
        GET_CONF_ITEM_INT(breakcontrol,m_flowConfig.nBreakCtrl);
        GET_CONF_ITEM_INT(sleeptimems,m_flowConfig.nSleepTimeMs);
    }
    catch (NotFoundException& exc ) {
        cerr << "Flow LoadConfig:" << exc.displayText() << endl;
        return false;
    } catch (SyntaxException& exc) {
        cerr << "Flow LoadConfig:" << exc.displayText() << endl;
        return false;
    }

    return true;
}

void CFlowMgr::printConfig() {
    FLOW_INFO_S(<=========flow configure information=========>);
    FLOW_INFO_V(" log4configfile=>   ["+string(m_flowConfig.chLog4File)+string("]"));
    char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",m_flowConfig.nModuleId);
    FLOW_INFO_V(" moduleid=>         ["+string(chModuleId)+string("]"));
    FLOW_INFO_V(" modulename=>       ["+string(m_flowConfig.chModuleName)+string("]"));
    char chVersion[10] = {0};
    sprintf(chVersion,"%d",m_flowConfig.wModuleVersion);
    FLOW_INFO_V(" wModuleVersion=>   ["+string(chVersion)+string("]"));
    char chPort[6] = {0};
    sprintf(chPort,"%d",m_flowConfig.nListenPort);
    FLOW_INFO_V(" flowlistenport=> ["+string(chPort)+string("]"));
    FLOW_INFO_V(" record_separator=> ["+string(m_flowConfig.chRecordSep)+string("]"));
    FLOW_INFO_V(" field_separator=>  ["+string(m_flowConfig.chFieldSep)+string("]"));

    FLOW_INFO_S(<=========flow configure information=========>);
}

bool CFlowMgr::Load(void)
{
    return true;
}

//调用方下发数据
bool CFlowMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
    m_vecPolicy.clear();
    string str = m_flowConfig.chFieldSep;
    string_split(strPolicy,str,m_vecPolicy);
    vector<string>::iterator iter = m_vecPolicy.begin();
    FLOW_INFO_S(<=========FLOW POlOCY=========>);
    for(;iter!= m_vecPolicy.end();iter++)
        FLOW_INFO_V(" FLOW POlOCY =>" + *iter);
    FLOW_INFO_S(<=========FLOW POlOCY=========>);
    return true;
}

//调用方获取数据
void * CFlowMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//调用方释放获取到的内存
void CFlowMgr::FreeData(void * pData)
{

}

//set callback for report function
void CFlowMgr::SetReportData(pFunc_ReportData pCbReport)
{
    m_pFuncReportData = pCbReport;
}

//set call back for fetch function
void CFlowMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
    m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CFlowMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    bool bRet = false;

    if (m_pFuncReportData){
        bRet =  m_pFuncReportData(m_flowConfig.nModuleId,pDataType,pData,dwLength);
    }
    return bRet;
}

//Fetch Data
const char* CFlowMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//启动模块
bool CFlowMgr::Start(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
    //stop后会导致单例失效
    pServer = FlowUdpClientService::inistance();
    if( pServer == NULL ) return false;
    if( !m_bIsRunning) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        if (pthread_create(&p_thread_udp, &pa, OnSatrtUdpServer, this) != 0) {
            FLOW_ERROR_S(Create OnSatrtUdpServer Thread failed!);
            return false;
        }
		usleep(50000);// 50毫秒
        FLOW_ERROR_S(Create OnSatrtUdpServer Thread successed!);

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            FLOW_ERROR_S(Create OnReportHandle Thread failed!);
            return false;
        }
        FLOW_INFO_S(Create OnReportHandle Thread successed!);
    }
    usleep(50000);// 20毫秒，启动线程需要一些执行时间
    FLOW_INFO_S(CSysLogMgr::Start successed!);

    return true;
}

//停止模块
bool CFlowMgr::Stop(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
    //停UDP线程,状态由线程自己改变
    pServer->handle_close(pServer->get_handle(), 0);
    // m_bIsRunning状态在OnSatrtUdpServer中改变
    //ACE_Reactor::instance()->end_event_loop();
        FlowUdpClientService::pReactor->end_reactor_event_loop();

    FLOW_INFO_S(CSysLogMgr::Stop finished!);
    return true;
}

void *CFlowMgr::OnFetchHandle(void *arg) {
    return NULL;
}

//开启UDP监听以及I/O事件检测
void* CFlowMgr::OnSatrtUdpServer(void *arg)
{
    CFlowMgr *cThis = reinterpret_cast<CFlowMgr*>(arg);

    //ACE_Reactor* pReactor = ACE_Reactor::instance();
    //cThis->pServer->reactor(pReactor);
       FlowUdpClientService::pReactor = new ACE_Reactor( (ACE_Reactor_Impl *)new ACE_TP_Reactor(), true);
    if( cThis->pServer->open(cThis->m_flowConfig.nListenPort) != 0 ) {
        cThis->pServer->handle_close(ACE_INVALID_HANDLE, 0);
        cThis->m_bIsRunning = true;
        FLOW_ERROR_S(OnSatrtUdpServer end ...);
        return NULL;
    }
	
	cThis->m_bUdpServerState = true;
    cThis->pServer->pReactor->run_reactor_event_loop(); //阻塞，内部循环
    FLOW_INFO_S(run_reactor_event_loop is ended !);

    cThis->pServer->pReactor->close();
    sleep(1);
	cThis->m_bUdpServerState = false;
	
    if ( cThis->pServer->pReactor != NULL )
        delete cThis->pServer->pReactor;
    cThis->pServer->pReactor = NULL;


    FLOW_INFO_S(OnStartUdpServer end ...);
    return NULL;
}

//获取日志
void *CFlowMgr::OnReportHandle(void *arg)
{
    CFlowMgr *cThis = reinterpret_cast<CFlowMgr*>(arg);

	while(true) {
		if (cThis->m_bUdpServerState) {  // udp 线程启动后，插件状态才更新为true
			cThis->m_bIsRunning = true;
		}
		bool  bReportCtrl = false;
        while( cThis->m_bUdpServerState && cThis->m_bIsRunning) {
            static int num = 0;
            if ( cThis->pServer->m_strLogList.size() >  cThis->m_flowConfig.nListCtrl || bReportCtrl) {
                int nBreakCtrl = 0;
                cThis->pServer->m_inputMutex.lock();
                list<string>::iterator iter = cThis->pServer->m_strLogList.begin();
                for(;iter != cThis->pServer->m_strLogList.end();) {
                    // 过滤策略中指定 IP 设备日志
                    //string::size_type firstpos = (*iter).find(cThis->m_flowConfig.chFieldSep);
                    //string strSrcIP = (*iter).substr(firstpos+1,(*iter).find(cThis->m_flowConfig.chFieldSep,firstpos+1)-firstpos-1);
                    //if(find(cThis->m_vecPolicy.begin(),cThis->m_vecPolicy.end(),strSrcIP) == cThis->m_vecPolicy.end()) {
                        modintf_datatype_t temp;
                        temp.nDataType     = MODULE_DATA_FLOW;
                        temp.nMainCategory = MODULE_DATA_FLOW_FLOW;
                        FLOW_DEBUG_V("flow:"+(*iter));
                        cThis->m_pFuncReportData(cThis->m_flowConfig.nModuleId,&temp,(void *)&(*iter),iter->length());
                    //}
                        ++nBreakCtrl;
                    iter = cThis->pServer->m_strLogList.erase(iter);
                    if (nBreakCtrl > cThis->m_flowConfig.nBreakCtrl) break;
                }
                cThis->pServer->m_inputMutex.unlock();
                num = 0;
            }
                if ( num == 0) {
                     num = cThis->pServer->m_strLogList.size();
                     bReportCtrl =false;
                } else  if( num == cThis->pServer->m_strLogList.size()){
                    bReportCtrl = true;
                }
                usleep(cThis->m_flowConfig.nSleepTimeMs * 1000);
        }
		if ( !cThis->m_bUdpServerState && cThis->m_bIsRunning) { // dup 线程退出了
			break;
		}
        sleep(1);
	}
	cThis->pServer->Destroy();
	cThis->m_bIsRunning = false;
    FLOW_INFO_S(OnReportHandle end ...);
    return NULL;
}

//base64解码
string CFlowMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);

  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));

  return out.str();
}


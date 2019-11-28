
#include "ScanPortMgr.h"
#include "utils/bdstring.h"
#include "port/bd_port.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CScanPortMgr::CScanPortMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_pingConfig,0,sizeof(m_pingConfig));
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;

	m_vecPolicy.clear();
	//m_vecThreadPool.clear();
}

CScanPortMgr::~CScanPortMgr(void)
{
	m_vecPolicy.clear();
	//m_vecThreadPool.clear();
}


//初始化数据，先于Start被调用
bool CScanPortMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_pingConfig.chLog4File) > 0 ) {
	    if(access(m_pingConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_pingConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    }
	else {
        return false;
	}
	
    SCANPORT_INFO_S( CScanPortMgr::Init() successed...);
	printConfig();

    return true;
	
}

bool CScanPortMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CScanPortMgr::GetModuleId(void) 
{
	return m_pingConfig.nModuleId;
}
UInt16 CScanPortMgr::GetModuleVersion(void) 
{
	return m_pingConfig.wModuleVersion;
}
string CScanPortMgr::GetModuleName(void) 
{
	return m_pingConfig.chModuleName;
}

//开始（下发)任务
bool CScanPortMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CScanPortMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CScanPortMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "PING LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "PING LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_pingConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_pingConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_pingConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_pingConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_pingConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_pingConfig.chFieldSep,2);
	}
	catch (NotFoundException& exc ) {
		cerr << "PING LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "PING LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

    return true;
}

void CScanPortMgr::printConfig() {
	SCANPORT_INFO_S(<=========netservice configure information=========>);
	SCANPORT_INFO_V(" log4configfile=>   ["+string(m_pingConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_pingConfig.nModuleId);
    SCANPORT_INFO_V(" moduleid=>         ["+string(chModuleId)+string("]"));
	SCANPORT_INFO_V(" modulename=>       ["+string(m_pingConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_pingConfig.wModuleVersion);
	SCANPORT_INFO_V(" wModuleVersion=>   ["+string(chVersion)+string("]"));
	SCANPORT_INFO_V(" record_separator=> ["+string(m_pingConfig.chRecordSep)+string("]"));
	SCANPORT_INFO_V(" field_separator=>  ["+string(m_pingConfig.chFieldSep)+string("]"));

	SCANPORT_INFO_S(<=========netservice configure information=========>);
}

bool CScanPortMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CScanPortMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	vector<string> vecRecord;
	string strRsep = m_pingConfig.chRecordSep;
	string strFsep = m_pingConfig.chFieldSep;
	
	string_split(strPolicy,strRsep,vecRecord);
	vector<string>::iterator iterR = vecRecord.begin();
	SCANPORT_INFO_S(<=======SCANPORT PlOCY=======>);
	for(;iterR != vecRecord.end(); iterR++) {
		vector<string> vecField;
		string_split(*iterR,strFsep,vecField);
		if( vecField.size() != 3 ) {
			SCANPORT_WARN_V("Invalid Polocy =>" + *iterR);
			continue; //策略格式不正确	
		}
		SCANPORT_INFO_V(" SCANPORT PlOCY =>" + *iterR);
		vector<string>::iterator iterF = vecField.begin();
		tag_canport_policy_t temp;
		memset(&temp, 0, sizeof(temp));
		strcpy(temp.chIP, (*iterF).c_str());
		++iterF;
		temp.sin_port_beg = atoi((*iterF).c_str());
		++iterF;	
		temp.sin_port_end = atoi((*iterF).c_str());
	    m_vecPolicy.push_back(temp);			
	}	
	SCANPORT_INFO_S(<=======SCANPORT PlOCY=======>);
	
	return true;
}

//调用方获取数据
void * CScanPortMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CScanPortMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CScanPortMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CScanPortMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CScanPortMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_pingConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CScanPortMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CScanPortMgr::Start(void)
{
    //string test = "172.16.12.153~1530~1570|172.16.12.211~~";
	//SetData(NULL,&test,0);

	FastMutex::ScopedLock lock(m_ModMutex);

	if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			SCANPORT_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		SCANPORT_INFO_S(Create OnReportHandle Thread successed!);
	}
	//usleep(10000);// 10毫秒，启动线程需要一些执行时间
	SCANPORT_INFO_S(CScanPortMgr::Start successed!);
	return true;
}

//停止模块
bool CScanPortMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
    m_bIsRunning = false; //只有单个线程，就直接在这里设置为结束
	SCANPORT_INFO_S(CScanPortMgr::Stop successed!);
    return true;
}

void *CScanPortMgr::OnFetchHandle(void *arg) {
	return NULL;
}

void *CScanPortMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CScanPortMgr *cThis = reinterpret_cast<CScanPortMgr*>(arg);
    cThis->m_bIsRunning = true;
    while(cThis->m_bIsRunning) {	
		//UInt32 dwRetLen = 0;
		vector<tag_canport_policy_t>::iterator itor_Tmp = cThis->m_vecPolicy.begin();
		for( ;itor_Tmp != cThis->m_vecPolicy.end(); itor_Tmp++) {
			if(!cThis->m_bIsRunning) break;
            for(int i= itor_Tmp->sin_port_beg; i<=itor_Tmp->sin_port_end;i++) {
			    string strIP = itor_Tmp->chIP;	
		        bool bConnect = CScanPort::checkPort(strIP, i); // check tcp port
		        if ( ! bConnect ) continue; // 只发成功链接的
		        char chPort[5] = {0};
				sprintf(chPort, "%d", i);
			    string strResult = strIP + cThis->m_pingConfig.chFieldSep + string(chPort);
				modintf_datatype_t temp;
				temp.nDataType     = MODULE_DATA_PERFORMANCE;
				temp.nMainCategory = MODULE_DATA_PERFORMANCE_SCANPORT;
				if(! cThis->ReportData(&temp,(void *)&strResult,strResult.length()) ) {
				    SCANPORT_INFO_S(Report data failed!);
	            } else {
			        SCANPORT_INFO_S(Report data success!);
	            }
				if(!cThis->m_bIsRunning) break;
            }			
		}
		cThis->m_vecPolicy.clear();
		/* 目前业务只是一次性业务，不过为了保持插件一致性,
		   同时考虑到以后业务应该改为持续性业务更为合理，
		   插件的暂时停止还是由外部触发。
		*/
		sleep(1); 
	}
	SCANPORT_INFO_S(OnReportHandle end ...);
    return NULL;
}

//base64解码
string CScanPortMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



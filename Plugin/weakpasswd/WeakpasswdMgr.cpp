
#include "WeakpasswdMgr.h"
#include "../../utils/bdstring.h"
#include "../share/weakpasswd/bd_weakpasswd_interface.h"
#include  "unistd.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CWeakpasswdMgr::CWeakpasswdMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_weakpasswdConfig,0,sizeof(m_weakpasswdConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CWeakpasswdMgr::~CWeakpasswdMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}


//初始化数据，先于Start被调用
bool CWeakpasswdMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_weakpasswdConfig.chLog4File) > 0 ) {
	    if(access(m_weakpasswdConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_weakpasswdConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    }
	else {
        return false;
	}
	
    WEAKPASSWD_INFO_S( CWeakpasswdMgr::Init() successed...);
	printConfig();

    return true;
	
}

bool CWeakpasswdMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CWeakpasswdMgr::GetModuleId(void) 
{
	return m_weakpasswdConfig.nModuleId;
}
UInt16 CWeakpasswdMgr::GetModuleVersion(void) 
{
	return m_weakpasswdConfig.wModuleVersion;
}
string CWeakpasswdMgr::GetModuleName(void) 
{
	return m_weakpasswdConfig.chModuleName;
}

//开始（下发)任务
bool CWeakpasswdMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CWeakpasswdMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CWeakpasswdMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "WEAKPASSWD LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "WEAKPASSWD LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_weakpasswdConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_weakpasswdConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_weakpasswdConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_weakpasswdConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_weakpasswdConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_weakpasswdConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_weakpasswdConfig.nTaskNumPerThread);
        GET_CONF_ITEM_INT(nSleepsec,m_weakpasswdConfig.nSleepsec);
		if( m_weakpasswdConfig.nTaskNumPerThread <= 0 ) {
            m_weakpasswdConfig.nTaskNumPerThread = 100;  // 默认每个线程处理5个IP
		}
        if( m_weakpasswdConfig.nSleepsec <= 0 ) {
            m_weakpasswdConfig.nSleepsec = 60;  // 默认睡眠60秒
        }
	}
	catch (NotFoundException& exc ) {
		cerr << "WEAKPASSWD LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "WEAKPASSWD LoadConfig:" << exc.displayText() << endl;	
		return false;
	}
    return true;
}

void CWeakpasswdMgr::printConfig() {
	WEAKPASSWD_INFO_S(<=========netservice configure information=========>);
	WEAKPASSWD_INFO_V(" log4configfile=>    ["+string(m_weakpasswdConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_weakpasswdConfig.nModuleId);
    WEAKPASSWD_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	WEAKPASSWD_INFO_V(" modulename=>        ["+string(m_weakpasswdConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_weakpasswdConfig.wModuleVersion);
	WEAKPASSWD_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	WEAKPASSWD_INFO_V(" record_separator=>  ["+string(m_weakpasswdConfig.chRecordSep)+string("]"));
	WEAKPASSWD_INFO_V(" field_separator=>   ["+string(m_weakpasswdConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_weakpasswdConfig.nTaskNumPerThread);
	WEAKPASSWD_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    sprintf(chTemp,"%d",m_weakpasswdConfig.nSleepsec);
    WEAKPASSWD_INFO_V(" nSleepsec=> ["+string(chTemp)+string("]"));
	WEAKPASSWD_INFO_S(<=========netservice configure information=========>);
}

bool CWeakpasswdMgr::Load(void)
{
	return true;
}

//调用方下发数据  TODO
bool CWeakpasswdMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();	
	vector<string> vecIP;
	string strFsep = m_weakpasswdConfig.chFieldSep;
	string_split(strPolicy,strFsep,vecIP);
    m_vecPolicy.reserve(vecIP.size());

    WEAKPASSWD_INFO_S(<=========WEAKPASSWD PlOCY=========>);
    vector<string>::iterator iter = vecIP.begin();
	for(;iter!= vecIP.end();iter++) {
		tag_weakpasswd_policy_t temp;
		temp.strIP   = *iter;
        //temp.nResult = 0;
        //temp.bChange = false;
		m_vecPolicy.push_back(temp);
		WEAKPASSWD_INFO_V(" WEAKPASSWD PlOCY =>" + temp.strIP);
	}
	WEAKPASSWD_INFO_S(<=========WEAKPASSWD PlOCY=========>);

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_weakpasswdConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间

	return true;
}

//调用方获取数据
void * CWeakpasswdMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CWeakpasswdMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CWeakpasswdMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CWeakpasswdMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CWeakpasswdMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_weakpasswdConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CWeakpasswdMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块  TODO
bool CWeakpasswdMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);

    /* for test.....
     *string test = "172.16.12.188~172.16.12.43~172.16.33.4~172.16.12.17";
	 *SetData(NULL,&test,0);
	 */
    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
			tag_weakpasswd_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnWeakpasswdHandle, this) != 0) {
				WEAKPASSWD_ERROR_S(Create OnWeakpasswdHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
			WEAKPASSWD_INFO_S(Create OnWeakpasswdHandle Thread successed!);
		}
		
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			WEAKPASSWD_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		WEAKPASSWD_INFO_S(Create OnReportHandle Thread successed!);	
	}

	m_bIsLoaded = true;
	WEAKPASSWD_INFO_S(CWeakpasswdMgr::Start successed!);
	return true;
}

//停止模块  DONE
bool CWeakpasswdMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}
        //stopAllWorkspace();
	//m_vecThreadPool
    //m_bIsRunning = false;
	WEAKPASSWD_INFO_S(CWeakpasswdMgr::Stop successed!);
    return true;
}

void *CWeakpasswdMgr::OnFetchHandle(void *arg) {

	return NULL;
}

//上报任务处理结果  DONE
void *CWeakpasswdMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CWeakpasswdMgr *cThis = reinterpret_cast<CWeakpasswdMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_weakpasswd_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {	
            modintf_datatype_t temp;
            temp.nDataType	   = MODULE_DATA_PERFORMANCE;
            temp.nMainCategory = MODULE_DATA_PERFORMANCE_WEAKPASSWD;
            if( iter_temp->bFinished ) { // 任务完成才上报
                if ( iter_temp->strResult.length() > 0 ){
                    if(! cThis->ReportData(&temp,(void *)&(iter_temp->strResult),iter_temp->strResult.length()) ) {
                        WEAKPASSWD_DEBUG_V(string("Report data failed!") + iter_temp->strResult);
                    } else {
                        WEAKPASSWD_DEBUG_S(Report data success!);
                    }
                    //成功或者失败都更新状态
                    iter_temp->bFinished = false;
                    iter_temp->bReported = true;
                }
			}
        }		
		if(cThis->m_pthreadnum_alive == 0 ) {
			break; //weakpasswd处理线程池全部退出后则退出该线程
		}
        if(cThis->m_bIsRunning) sleep(5); // 不用一直扫描
    }
	cThis->m_bIsRunning = false;
	cThis->m_vecThreadPool.clear();
 	WEAKPASSWD_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}
//模块处理函数  DONE
void* CWeakpasswdMgr::OnWeakpasswdHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CWeakpasswdMgr *cThis = reinterpret_cast<CWeakpasswdMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动

	int nPolocyNum = cThis->m_vecPolicy.size();

	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning ) {		
            WeakpasswdInfoVec vecWeakpasswd;
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
                int ret = GetWeakPassword(cThis->m_vecPolicy[i].strIP.c_str(), vecWeakpasswd);
                if(ret != 0)
                    WEAKPASSWD_ERROR_V(cThis->m_vecPolicy[i].strIP + "GetWeakPassword failed!");

                if ( !cThis->m_vecPolicy[i].bFinished ) {

                    int nWeekpwdNum = vecWeakpasswd.size();
                    char strWeekpwdNum[32];
                    sprintf(strWeekpwdNum, "%d", nWeekpwdNum);

                    string strWeekpwdRes;
                    vector<WeakpasswdInfo>::iterator iter = vecWeakpasswd.begin();
                    for(int iNum = 1; iter != vecWeakpasswd.end(); iter++)
                    {
                        strWeekpwdRes += iter->servicenm;
                        strWeekpwdRes += cThis->m_weakpasswdConfig.chFieldSep;
                        strWeekpwdRes += iter->username;
                        strWeekpwdRes += cThis->m_weakpasswdConfig.chFieldSep;
                        strWeekpwdRes += iter->pwd;

                        if(nWeekpwdNum != iNum++)
                            strWeekpwdRes += cThis->m_weakpasswdConfig.chRecordSep;
                    }

                    // IP~弱密码数量|服务名称~软件用户名~用户密码|服务名称~软件用户名~用户密码|...
                    string strResult = cThis->m_vecPolicy[i].strIP;
                    strResult += cThis->m_weakpasswdConfig.chFieldSep;
                    strResult += strWeekpwdNum;
                    strResult += cThis->m_weakpasswdConfig.chRecordSep;
                    strResult += strWeekpwdRes;

                    WEAKPASSWD_DEBUG_V("Report Data: " + strResult);

                    cThis->m_vecPolicy[i].strResult = strResult;
                    cThis->m_vecPolicy[i].bFinished = true; //任务执行完成
                }

                usleep(10 * 1000); //休眠10ms
                if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断继续执行
                    break;             
			}

            for(int i = 0; i < cThis->m_weakpasswdConfig.nSleepsec/5; i++)
            {
                sleep(5);
                if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                    break;
            }
		}
	}
	--cThis->m_pthreadnum_alive;
 	WEAKPASSWD_INFO_S(OnWeakpasswdHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CWeakpasswdMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



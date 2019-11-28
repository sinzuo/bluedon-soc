
#include "inspecUrlMgr.h"
#include "common/BDScModuleBase.h"
#include "utils/bdstring.h"
#include "https/bd_https.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;
using namespace netprotocol;


CInspecUrlMgr::CInspecUrlMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_inspecUrlConfig,0,sizeof(m_inspecUrlConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CInspecUrlMgr::~CInspecUrlMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CInspecUrlMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_inspecUrlConfig.chLog4File) > 0 ) {
	    if(access(m_inspecUrlConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_inspecUrlConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    }
    else {
        return false;
	}
	
    INSPECURL_INFO_S( CInspecUrlMgr::Init() successed...);
	printConfig();

    return true;	
}

bool CInspecUrlMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CInspecUrlMgr::GetModuleId(void) 
{
	return m_inspecUrlConfig.nModuleId;
}
UInt16 CInspecUrlMgr::GetModuleVersion(void) 
{
	return m_inspecUrlConfig.wModuleVersion;
}
string CInspecUrlMgr::GetModuleName(void) 
{
	return m_inspecUrlConfig.chModuleName;
}

//开始（下发)任务
bool CInspecUrlMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}
//停止（取消）任务
bool CInspecUrlMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

//导入配置文件
bool CInspecUrlMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "INSPECURL LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "INSPECURL LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_inspecUrlConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_inspecUrlConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_inspecUrlConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_inspecUrlConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_inspecUrlConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_inspecUrlConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_inspecUrlConfig.nTaskNumPerThread);
		if( m_inspecUrlConfig.nTaskNumPerThread <= 0 ) {
			m_inspecUrlConfig.nTaskNumPerThread = 1;  // 默认每个线程处理1个任务
		}
	}
	catch (NotFoundException& exc ) {
		cerr << "INSPECURL LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "INSPECURL LoadConfig:" << exc.displayText() << endl;	
		return false;
	}
    return true;
}
//输出配置信息到日志文件, 在Init()函数中被调用
void CInspecUrlMgr::printConfig() {
	INSPECURL_INFO_S(<=========netservice configure information=========>);
	INSPECURL_INFO_V(" log4configfile=>    ["+string(m_inspecUrlConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_inspecUrlConfig.nModuleId);
    INSPECURL_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	INSPECURL_INFO_V(" modulename=>        ["+string(m_inspecUrlConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_inspecUrlConfig.wModuleVersion);
	INSPECURL_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	INSPECURL_INFO_V(" record_separator=>  ["+string(m_inspecUrlConfig.chRecordSep)+string("]"));
	INSPECURL_INFO_V(" field_separator=>   ["+string(m_inspecUrlConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_inspecUrlConfig.nTaskNumPerThread);
	INSPECURL_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
	INSPECURL_INFO_S(<=========netservice configure information=========>);
}

bool CInspecUrlMgr::Load(void)
{
	return true;
}

//调用方下发数据  DONE
bool CInspecUrlMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
	vector<string> vec_task_unit;
	string strRsep = m_inspecUrlConfig.chRecordSep;
	string_split(strPolicy,strRsep,vec_task_unit,false);
	
	INSPECURL_INFO_S(<=======INSPECURL PlOCY=======>);
	INSPECURL_INFO_V(" INSPECURL POlOCY =>" + strPolicy);
	
    //防止因数据策略失败而导致段错误
    if(vec_task_unit.size() == 0)
    {
        INSPECURL_ERROR_S(Invalid polocy length_error...);
        return false;
    }

	int nTaskNum = atoi(vec_task_unit[0].c_str());
	if( vec_task_unit.size() != (nTaskNum * 4 + 1)) {
		INSPECURL_ERROR_S(Invalid polocy string...);
		return false;
	}
    for( int i=0; i<nTaskNum; i++) {
		tag_inspecUrl_policy_t tag_policy;
		vector<string> vec_temp;
		string strFsep = m_inspecUrlConfig.chFieldSep;
		// 任务ID~任务类型(1:一次性任务)
		string_split(vec_task_unit[i*4 + 1],strFsep,vec_temp,false);
		if( vec_temp.size() != 2) {
			INSPECURL_ERROR_S(Invalid polocy string...);
			return false;
		} else {
			tag_policy.strTaskId = vec_temp[0];
			if( atoi(vec_temp[1].c_str()) == 1) tag_policy.bOnce = true; // 一次性任务
		}
		// 分~时~日~月~周
		tag_policy.strTimePolocy = vec_task_unit[i*4 + 2];
        // URL数量
        tag_policy.strUrlNum = vec_task_unit[i*4 + 3];
        // 参数字符串(URL1~URL2~...)
		string_split(vec_task_unit[i*4 + 4],strFsep,tag_policy.vec_options);
        int urlNum = atoi(tag_policy.strUrlNum.c_str());
        if(urlNum == 0) INSPECURL_ERROR_S(Get Data error in vec_options!);
        if(tag_policy.vec_options.size() != urlNum)
            return false;

        m_vecPolicy.push_back(tag_policy);
    }
    INSPECURL_INFO_S(<=======INSPECURL PlOCY=======>);

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_inspecUrlConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}

//调用方获取数据
void * CInspecUrlMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CInspecUrlMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CInspecUrlMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CInspecUrlMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CInspecUrlMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_inspecUrlConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CInspecUrlMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块  TODO
bool CInspecUrlMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
    //string str = "1|888888~1|*~*~*~*~*|172.16.2.144|2~https://172.16.2.144:441/cgi-bin/admin/bdsec_daemon"~https://www.baidu.com";
	//SetData(NULL,&str,str.length());

    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
			tag_inspecUrl_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnInspecUrlHandle, this) != 0) {
                  INSPECURL_ERROR_S(Create OnInspecUrlHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
			INSPECURL_INFO_S(Create OnInspecUrlHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			INSPECURL_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		INSPECURL_INFO_S(Create OnReportHandle Thread successed!);	
	}	
	m_bIsLoaded = true;
	INSPECURL_INFO_S(CInspecUrlMgr::Start successed!);
	return true;
}

//停止模块 
bool CInspecUrlMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
    }
    m_bIsRunning = false;
	INSPECURL_INFO_S(CInspecUrlMgr::Stop successed!);
    return true;
}

void *CInspecUrlMgr::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果  DONE
void *CInspecUrlMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CInspecUrlMgr *cThis = reinterpret_cast<CInspecUrlMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_inspecUrl_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_INSPECURL;	
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i<nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成才上报	
					if (iter_temp->bOnce && iter_temp->m_mapReported[i]) continue;	// 一次性任务而且已经上报过了	
					if ( iter_temp->m_mapResult[i].length() > 0 ){
						if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
							INSPECURL_DEBUG_S(Report data failed!);
						} else {
							INSPECURL_DEBUG_S(Report data success!);
						}
                        //成功或者失败都更新状态
						iter_temp->m_mapFinished[i] = false;
						iter_temp->m_mapReported[i] = true;
					}
				}	
            }
        }		
		if(cThis->m_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程	
		if(cThis->m_bIsRunning) sleep(1);  // 不用一直扫描
    }
	cThis->m_bIsRunning = false;	
	cThis->m_vecThreadPool.clear();
 	INSPECURL_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}
//InspecUrl模块处理函数  DONE
void* CInspecUrlMgr::OnInspecUrlHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CInspecUrlMgr *cThis = reinterpret_cast<CInspecUrlMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {	
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
               bool bReady = check_cron(cThis->m_vecPolicy[i].strTimePolocy);
			  if (!bReady) continue;
				 
                //CTask InspecUrlTask(cThis->m_vecPolicy[i].vec_options);
                if ( !cThis->m_vecPolicy[i].m_mapFinished[0] ) { // inspecUrl
                    INSPECURL_INFO_V(string("Ready to test Url, id: ") + cThis->m_vecPolicy[i].strTaskId);

                    int urlNUm = atoi(cThis->m_vecPolicy[i].strUrlNum.c_str());
                    if(urlNUm == 0)
                        INSPECURL_ERROR_S(Can not get url number!);

                    string strUrlRes;
                    bd_https http;
                    for(int j = 0; j < urlNUm; j++)
                    {
                        int nCostTime = 0; //ms
                        int res = http.test_request_page(cThis->m_vecPolicy[i].vec_options[j], &nCostTime);

                        char resState[8];
                        sprintf(resState, "%d", res);  //CURLE_OK值为0
                        strUrlRes += resState;
                        strUrlRes += cThis->m_inspecUrlConfig.chFieldSep;

                        char chCostTime[8];
                        sprintf(chCostTime, "%d", nCostTime);
                        strUrlRes += chCostTime;

                        if(j < urlNUm - 1) strUrlRes += cThis->m_inspecUrlConfig.chRecordSep;
                    }

                    // 任务标识~URL数量|URL1结果标识(0：能访问; 非0：不能访问)~响应时间(ms, 不能访问时,值为0)|URL2结果标识~响应时间...
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspecUrlConfig.chFieldSep;
                    strResult += cThis->m_vecPolicy[i].strUrlNum;
                    strResult += cThis->m_inspecUrlConfig.chRecordSep;
                    strResult += strUrlRes;
					
                    INSPECURL_DEBUG_V("Report Data: " + strResult);

					cThis->m_vecPolicy[i].m_mapResult[0] = strResult;
                    cThis->m_vecPolicy[i].m_mapFinished[0] = true; //任务执行完成
				}

                if ( (i + cThis->m_pthreadnum) >= nPolocyNum ) {
					/* 
					 当前线程被分配的最后一个任务已处理完;
					 任务启动时间检验单位为: 分钟;
					*/
                    sleep(20);
				}
			}
            sleep(10);
		}
	}
	--cThis->m_pthreadnum_alive;
 	INSPECURL_INFO_S(OnInspecUrlHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CInspecUrlMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



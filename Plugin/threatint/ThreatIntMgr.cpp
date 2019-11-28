
#include "ThreatIntMgr.h"
#include "utils/bdstring.h"
#include "https/bd_https.h"
#include "curl/curl.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;
using namespace netprotocol;


CThreatInt::CThreatInt(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_threadintConfig,0,sizeof(m_threadintConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CThreatInt::~CThreatInt(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CThreatInt::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_threadintConfig.chLog4File) > 0 ) {
	    if(access(m_threadintConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_threadintConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    THREATINT_INFO_S( CThreatInt::Init() successed...);
	printConfig();

    return true;	
}

bool CThreatInt::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CThreatInt::GetModuleId(void) 
{
	return m_threadintConfig.nModuleId;
}
UInt16 CThreatInt::GetModuleVersion(void) 
{
	return m_threadintConfig.wModuleVersion;
}
string CThreatInt::GetModuleName(void) 
{
	return m_threadintConfig.chModuleName;
}

//开始（下发)任务
bool CThreatInt::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CThreatInt::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CThreatInt::LoadConfig(void)
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
		GET_CONF_ITEM_CHAR(log4configfile,m_threadintConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_threadintConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_threadintConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_threadintConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_threadintConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_threadintConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_threadintConfig.nTaskNumPerThread);
		if( m_threadintConfig.nTaskNumPerThread <= 0 ) {
			m_threadintConfig.nTaskNumPerThread = 5;  // 默认每个线程处理5个IP
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

void CThreatInt::printConfig() {
	THREATINT_INFO_S(<=========netservice configure information=========>);
	THREATINT_INFO_V(" log4configfile=>    ["+string(m_threadintConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_threadintConfig.nModuleId);
    THREATINT_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	THREATINT_INFO_V(" modulename=>        ["+string(m_threadintConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_threadintConfig.wModuleVersion);
	THREATINT_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	THREATINT_INFO_V(" record_separator=>  ["+string(m_threadintConfig.chRecordSep)+string("]"));
	THREATINT_INFO_V(" field_separator=>   ["+string(m_threadintConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_threadintConfig.nTaskNumPerThread);
	THREATINT_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
	THREATINT_INFO_S(<=========netservice configure information=========>);
}

bool CThreatInt::Load(void)
{
	return true;
}

//调用方下发数据
bool CThreatInt::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;   
	m_vecPolicy.clear();		
    vector<string> vec_task_unit;
	string strFsep = m_threadintConfig.chFieldSep;
    string strRsep = m_threadintConfig.chRecordSep;
    string_split(strPolicy,strRsep,vec_task_unit,false);

    THREATINT_INFO_S(<=======THREATINT PlOCY=======>);
    for( int i = 0; i < vec_task_unit.size(); i++)
    {
        tag_threadint_policy_t tag_policy;
        vector<string> vec_temp;
        THREATINT_INFO_V("THREATINT POlOCY => " + vec_task_unit[i]);
        //标识ID1~厂家(1.微步 2.IBM)~url1~请求方式(post, get)~请求参数(没有则填 "null")~header参数(用户名:密码, 没有则填 "null")|...
        string_split(vec_task_unit[i],strFsep,vec_temp);
        tag_policy.vec_options = vec_temp;
        if(vec_temp.size() != 6)
        {
            m_vecPolicy.clear();
            THREATINT_ERROR_S(Policy is Unavailable...);
            continue;
        }
        m_vecPolicy.push_back(tag_policy);
    }

    THREATINT_INFO_S(<=======THREATINT PlOCY=======>);
	
    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_threadintConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}

//调用方获取数据
void * CThreatInt::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CThreatInt::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CThreatInt::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CThreatInt::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CThreatInt::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_threadintConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CThreatInt::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

std::string CThreatInt::quest_for_result(std::vector<std::string> &vecParameter) //TODO
{
    string result = "0~";

    if(vecParameter.size() != 4) {
        THREATINT_ERROR_S(vecParameter size error...);
        return string("1~null");
    }
    string command = vecParameter[1];
    string curl = vecParameter[0];
    string curlPara = vecParameter[2];
    string header = vecParameter[3];

    bd_https http;

    if(command.compare("post") == 0 && curlPara.compare("null") != 0)
    {
        if(header.compare("null") == 0)
        {
            try {
                result += http.post_request_page(curl, curlPara);
            }
            catch (...) {
                THREATINT_ERROR_S(post_request_page() error...);
                result = "1~null";
            }
        }
    } else if(command.compare("get") == 0)
    {
        if(curlPara.compare("null") == 0) curlPara = "";
        else
        {
            curlPara = "?" + curlPara;
        }

        string newcurl = curl + curlPara;
        if(header.compare("null") == 0)
        {
            try {
                result += http.get_request_page(newcurl);
            }
            catch (...) {
                THREATINT_ERROR_S(get_request_page() error...);
                result = "1~null";
            }
        }
    } else
    {
        THREATINT_ERROR_S(Unkown command detected...);
        result = "1~null";
    }

    return result;
}

//启动模块
bool CThreatInt::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	   
	//SetData(NULL,&str,str.length());
    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        m_vecThreadPool.clear();
        for (int i=0; i < m_pthreadnum; i++) {
			tag_threadint_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnThreatIntHandle, this) != 0) {
				THREATINT_ERROR_S(Create OnThreatIntHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
            THREATINT_INFO_S(Create OnThreatIntHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			THREATINT_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		THREATINT_INFO_S(Create OnReportHandle Thread successed!);	
	}	
	
	m_bIsLoaded = true;
	THREATINT_INFO_S(CThreatInt::Start successed!);
	return true;
}

//停止模块
bool CThreatInt::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}
	THREATINT_INFO_S(CThreatInt::Stop successed!);
    return true;
}

void *CThreatInt::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CThreatInt::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CThreatInt *cThis = reinterpret_cast<CThreatInt*>(arg);

    if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
	while(true) {
        vector<tag_threadint_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_THREATINT;	
            int nChildTaskNum = iter_temp->m_mapFinished.size(); //m_mapFinished.size() = 1
            for(int i=0 ; i < nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成	
					if(!cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
                        THREATINT_ERROR_S(Report data failed!);
					} else {
						THREATINT_DEBUG_S(Report data success!);
                    }
                      
                    cThis->Stop();
					iter_temp->m_mapFinished[i] = false;  // 成功或失败下次都重新执行任务
				}	
            }
        }
		if(cThis->m_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程	
		if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
	cThis->m_bIsRunning = false;	
	cThis->m_vecThreadPool.clear();
 	THREATINT_INFO_S(OnReportHandle Thread exit...);
    return NULL;

}

void* CThreatInt::OnThreatIntHandle(void *arg) //TODO
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CThreatInt *cThis = reinterpret_cast<CThreatInt*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
    string result;
    while( cThis->m_vecThreadPool[thread_num].b_state)
    {
        if(cThis->m_bIsRunning)
        {
            for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum)
            {
                if(cThis->m_vecPolicy[i].m_mapFinished[0] == false)
                {
                    /* string str = "标识ID~厂家(1.微步 2.IBM)~url~请求方式(post, get) \
                     *              ~请求参数(如:用户名=aaa&密码=bbb, 没有则填 "null") \
                     *              ~header参数(如:用户名:aaa&密码:bbb, 没有则填 "null")";
                     */
                    vector<string> v_options_tmp = cThis->m_vecPolicy[i].vec_options;

                    v_options_tmp.erase(v_options_tmp.begin()); //删除标识ID结点
                    string object = v_options_tmp[0];
                    v_options_tmp.erase(v_options_tmp.begin()); //删除厂家结点

                    if(object.compare("1") == 0)
                    {
                        result = cThis->quest_for_result(v_options_tmp);

                    } else  //TODO
                    {
                        THREATINT_INFO_S(Unkown Sourcer...);
                    }


                    //threatinthd.threatintRelease();
                    //标识ID~请求结果标识(0：成功  非0：失败)~返回数据(如果请求失败, 则填"null")
                    string SrcReport = cThis->m_vecPolicy[i].vec_options[0];
                    SrcReport += cThis->m_threadintConfig.chFieldSep;
                    SrcReport += cThis->m_vecPolicy[i].vec_options[1];
                    SrcReport += cThis->m_threadintConfig.chFieldSep;

                    SrcReport += result;
                    cThis->m_vecPolicy[i].m_mapResult[0] = SrcReport;
                    cThis->m_vecPolicy[i].m_mapFinished[0] = true;
                    THREATINT_DEBUG_V("threatint result:"+SrcReport);
                } //end if(m_vecPolicy[i].m_mapFinished[0] == false)
                sleep(1);
            }
        } //if(m_bIsRunning)

        for(int i = 0; i < 300/5; ++i)
        {
            if(!cThis->m_vecThreadPool[thread_num].b_state) break;
            sleep(5);
        }
    }
    while(true)
    {
        if(!(cThis->m_vecThreadPool[thread_num].b_state))
        {
            --cThis->m_pthreadnum_alive;
            break;
        }
    }
 	THREATINT_INFO_S(OnThreatIntHandle Thread exit...);
    return NULL;
}

//base64解码
string CThreatInt::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}

// 11010012~1~https://x.threatbook.cn/api/v2/ip_reputation~post~ip=82.165.37.26&apikey=fc28026c8d6647dbbbb5edae5d1e44aa30ef0f5412db416a8e0d885d36ba77a4~null

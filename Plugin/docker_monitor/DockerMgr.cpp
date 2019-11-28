
#include "DockerMgr.h"
#include "utils/bdstring.h"
#include "https/bd_https.h"
#include "json_interface.h"
#include "utils/bd_common.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;
using namespace netprotocol;


CDockerMgr::CDockerMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_dockerConfig,0,sizeof(m_dockerConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CDockerMgr::~CDockerMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CDockerMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_dockerConfig.chLog4File) > 0 ) {
        if(access(m_dockerConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
                log4cxx::PropertyConfigurator::configure(m_dockerConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    DOCKER_INFO_S( CDockerMgr::Init() successed...);
	printConfig();

    return true;	
}

bool CDockerMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CDockerMgr::GetModuleId(void)
{
    return m_dockerConfig.nModuleId;
}
UInt16 CDockerMgr::GetModuleVersion(void)
{
    return m_dockerConfig.wModuleVersion;
}
string CDockerMgr::GetModuleName(void)
{
    return m_dockerConfig.chModuleName;
}

//开始（下发)任务
bool CDockerMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CDockerMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CDockerMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
        cerr << "DOCKER LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
        cerr << "DOCKER LoadConfig:" << exc.displayText() << endl;
		return false;
	}

	try {
        GET_CONF_ITEM_CHAR(log4configfile,m_dockerConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_dockerConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_dockerConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_dockerConfig.wModuleVersion);
        GET_CONF_ITEM_CHAR(record_separator,m_dockerConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_dockerConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(tasknum_perthread,m_dockerConfig.nTaskNumPerThread);
        if( m_dockerConfig.nTaskNumPerThread <= 0 ) {
            m_dockerConfig.nTaskNumPerThread = 2;  // 默认每个线程处理2个IP
		}
	}
	catch (NotFoundException& exc ) {
        cerr << "DOCKER LoadConfig:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
        cerr << "DOCKER LoadConfig:" << exc.displayText() << endl;
		return false;
	}
    return true;
}

void CDockerMgr::printConfig() {
    DOCKER_INFO_S(<=========netservice configure information=========>);
    DOCKER_INFO_V(" log4configfile=>    ["+string(m_dockerConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",m_dockerConfig.nModuleId);
    DOCKER_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
    DOCKER_INFO_V(" modulename=>        ["+string(m_dockerConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
    sprintf(chVersion,"%d",m_dockerConfig.wModuleVersion);
    DOCKER_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
    DOCKER_INFO_V(" record_separator=>  ["+string(m_dockerConfig.chRecordSep)+string("]"));
    DOCKER_INFO_V(" field_separator=>   ["+string(m_dockerConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
    sprintf(chTemp,"%d",m_dockerConfig.nTaskNumPerThread);
    DOCKER_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    DOCKER_INFO_S(<=========netservice configure information=========>);
}

bool CDockerMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CDockerMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
	vector<string> vec_task_unit;
    string strRsep = m_dockerConfig.chRecordSep;
	string_split(strPolicy,strRsep,vec_task_unit);
	
    DOCKER_INFO_S(<=======DOCKER PlOCY=======>);
	int nNum = vec_task_unit.size();  // 监控IP数量
    for( int i=0; i<nNum; i++) {
        DOCKER_INFO_V(" DOCKER POlOCY =>" + vec_task_unit[i]);
        tag_docker_policy_t tag_policy;
		vector<string> vec_temp;
        string strFsep = m_dockerConfig.chFieldSep;
        // IP~port~频率
		string_split(vec_task_unit[i],strFsep,vec_temp);
		
        if(vec_temp.size() != 3) {
            DOCKER_ERROR_S(Invalid polocy string...);
			return false;
        }
        else{
            tag_policy.vec_options = vec_temp;
           }
        m_vecPolicy.push_back(tag_policy);
    }
    DOCKER_INFO_S(<=======DOCKER PlOCY=======>);
	
    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_dockerConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}
//调用方获取数据
void * CDockerMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CDockerMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CDockerMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CDockerMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CDockerMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_dockerConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CDockerMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CDockerMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
            tag_docker_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnMonitorHandle, this) != 0) {
                DOCKER_ERROR_S(Create OnMonitorHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
            DOCKER_INFO_S(Create OnMonitorHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            DOCKER_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
        DOCKER_INFO_S(Create OnReportHandle Thread successed!);
	}	

	m_bIsLoaded = true;
    DOCKER_INFO_S(CDockerMgr::Start successed!);
	return true;
}

//停止模块
bool CDockerMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}	
    DOCKER_INFO_S(CDockerMgr::Stop successed!);
    return true;
}

void *CDockerMgr::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CDockerMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CDockerMgr *cThis = reinterpret_cast<CDockerMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_docker_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
            temp.nMainCategory = MODULE_DATA_PERFORMANCE_DOCKER;
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i < nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成	
					if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
                        DOCKER_DEBUG_S(Report data failed!);
					} else {
                        DOCKER_DEBUG_S(Report data success!);
					}
					iter_temp->m_mapFinished[i] = false;  // 成功或失败下次都重新执行任务
				}	
            }
        }		
		if(cThis->m_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程	
		if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
	cThis->m_bIsRunning = false;
	cThis->m_vecThreadPool.clear();
    DOCKER_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}

void* CDockerMgr::OnMonitorHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CDockerMgr *cThis = reinterpret_cast<CDockerMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {		
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
				 
                vector<string> v_options_tmp = cThis->m_vecPolicy[i].vec_options;

                if ( !cThis->m_vecPolicy[i].m_mapFinished[0] ) { //

                    bd_https http;
                    string content;
                    string content_new;
                    //获取多少个容器
                    string url =  "http://"+ v_options_tmp[0] + ":" + v_options_tmp[1] +"/containers/json?all=1";
                    content = http.get_request_page(url);

                    vector<containers_info> vec;
                    json_parse_result(content, vec);
                    vector<containers_info>::iterator iter;
                    DOCKER_INFO_V(string("Ready to get info  from : ") + url);
                    DOCKER_INFO_V(string("size of : ") + bd_int2str(vec.size()));


                   string strResult = cThis->m_vecPolicy[i].vec_options[0];
                   strResult += cThis->m_dockerConfig.chRecordSep;

                    for (iter = vec.begin(); iter != vec.end(); iter++)
                    {
                        //获取容器时间
                        url = "http://"+ v_options_tmp[0] + ":" + v_options_tmp[1] +"/containers/"+ iter->Id +"/json";
                        content = http.get_request_page(url);
                        container_json(content, iter);

                        url = "http://"+ v_options_tmp[0] + ":" + v_options_tmp[1] +"/containers/"+ iter->Id +"/stats?stream=false";
                        content = http.get_request_page(url);
                        content_new = http.get_request_page(url); //计算cpu占用，需要获取两次状态计算差值

                        inspect_result(content, content_new, iter);


                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "contain_id";
                        //strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->Id;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "image";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->Image;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "imageID";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->ImageID;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "state";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->State;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "cpu_count";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += bd_int2str(iter->cpu_count);
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "mem_limit";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += bd_double2str(iter->mem_limit);
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "cpu_percent";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += bd_double2str(iter->cpu_percent);
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "mem_percent";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += bd_double2str(iter->mem_percent);
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "Command";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->Command;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "NetworkMode";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->NetworkMode;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "IPAddress";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->IPAddress;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "MAC";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->MAC;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "Created";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->Created;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "StartedAt";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->StartedAt;
                        //strResult += cThis->m_dockerConfig.chRecordSep;
                        //strResult += "FinishedAt";
                        strResult += cThis->m_dockerConfig.chFieldSep;
                        strResult += iter->FinishedAt;
                        strResult += cThis->m_dockerConfig.chRecordSep;

                    }
                    strResult += "end";

                    DOCKER_INFO_V(string("Ready to send =>") + strResult);

					cThis->m_vecPolicy[i].m_mapResult[0] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[0] = true;

				}
            sleep(atoi(v_options_tmp[2].c_str()));
			}
            sleep(1);
		}
	}
	--cThis->m_pthreadnum_alive;
    DOCKER_INFO_S(OnMonitorHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CDockerMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



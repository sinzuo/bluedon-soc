
#include "PingMgr.h"
#include "utils/bdstring.h"
#include "icmp/bd_ping.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CPingMgr::CPingMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_pingConfig,0,sizeof(m_pingConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CPingMgr::~CPingMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}


//初始化数据，先于Start被调用
bool CPingMgr::Init(void)
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
	
    PING_INFO_S( CPingMgr::Init() successed...);
	printConfig();

    return true;
	
}

bool CPingMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CPingMgr::GetModuleId(void) 
{
	return m_pingConfig.nModuleId;
}
UInt16 CPingMgr::GetModuleVersion(void) 
{
	return m_pingConfig.wModuleVersion;
}
string CPingMgr::GetModuleName(void) 
{
	return m_pingConfig.chModuleName;
}

//开始（下发)任务
bool CPingMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CPingMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CPingMgr::LoadConfig(void)
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
	    GET_CONF_ITEM_INT(tasknum_perthread,m_pingConfig.nTaskNumPerThread);
        GET_CONF_ITEM_INT(nSleepsec,m_pingConfig.nSleepsec);
		if( m_pingConfig.nTaskNumPerThread <= 0 ) {
            m_pingConfig.nTaskNumPerThread = 1000;  // 默认每个线程处理1000个IP
		}
        if( m_pingConfig.nSleepsec <= 0 ) {
            m_pingConfig.nSleepsec = 60;  // 默认睡眠60秒
        }
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

void CPingMgr::printConfig() {
	PING_INFO_S(<=========netservice configure information=========>);
	PING_INFO_V(" log4configfile=>    ["+string(m_pingConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_pingConfig.nModuleId);
    PING_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	PING_INFO_V(" modulename=>        ["+string(m_pingConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_pingConfig.wModuleVersion);
	PING_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	PING_INFO_V(" record_separator=>  ["+string(m_pingConfig.chRecordSep)+string("]"));
	PING_INFO_V(" field_separator=>   ["+string(m_pingConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_pingConfig.nTaskNumPerThread);
	PING_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    sprintf(chTemp,"%d", m_pingConfig.nSleepsec);
    PING_INFO_V(" nSleepsec=> ["+string(chTemp)+string("]"));
	PING_INFO_S(<=========netservice configure information=========>);
}

bool CPingMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CPingMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();	
	vector<string> vecIP;
	
	string strFsep = m_pingConfig.chFieldSep;
	string_split(strPolicy,strFsep,vecIP);

    if(vecIP.size() == 0)
        return false;

    m_vecPolicy.reserve(vecIP.size());

	vector<string>::iterator iter = vecIP.begin();
	PING_INFO_S(<=========PING PlOCY=========>);
	for(;iter!= vecIP.end();iter++) {
		tag_ping_policy_t temp;
		temp.strIP   = *iter;
        //temp.nResult = 0;
		temp.bChange = false;
		m_vecPolicy.push_back(temp);
		PING_INFO_V(" PING PlOCY =>" + temp.strIP);
	}
	PING_INFO_S(<=========PING PlOCY=========>);

    m_pthreadnum = 1; //(int)ceil((double)m_vecPolicy.size()/m_pingConfig.nTaskNumPerThread);

	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间

    char chThreadNum[10] = {0};
    sprintf(chThreadNum, "%d", m_pthreadnum);
    PING_DEBUG_V("m_pthread number = " + string(chThreadNum));

	return true;
}

//调用方获取数据
void * CPingMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CPingMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CPingMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CPingMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CPingMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_pingConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CPingMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CPingMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);

    /* for test.....
     *string test = "172.16.12.153~172.16.33.3~172.16.33.4~172.16.12.210";
	 *SetData(NULL,&test,0);
	 */
    if (! m_bIsRunning ) {
		pthread_attr_t pa;       //线程属性
		pthread_attr_init(&pa);	//初始化线程属性
		pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
			tag_ping_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
			if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnPingHandle, this) != 0) {
				PING_ERROR_S(Create OnPingHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
			PING_INFO_S(Create OnPingHandle Thread successed!);
		}
		
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			PING_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		PING_INFO_S(Create OnReportHandle Thread successed!);	
	}

	m_bIsLoaded = true;
	PING_INFO_S(CPingMgr::Start successed!);
	return true;
}

//停止模块
bool CPingMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}
	//m_vecThreadPool
	
	PING_INFO_S(CPingMgr::Stop successed!);
    return true;
}

void *CPingMgr::OnFetchHandle(void *arg) {

	return NULL;
}

//上报ping结果
void *CPingMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CPingMgr *cThis = reinterpret_cast<CPingMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_ping_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {	
			if( iter_temp->bChange ) { // 结果有变化才上报
                char chRes[64] = {0};
                sprintf(chRes, "%d~%d", iter_temp->nResult, iter_temp->nTimecost);
				string strResult = chRes;
                strResult = iter_temp->strIP + cThis->m_pingConfig.chFieldSep + strResult + "~0~0";
				modintf_datatype_t temp;
				temp.nDataType     = MODULE_DATA_PERFORMANCE;
				temp.nMainCategory = MODULE_DATA_PERFORMANCE_PING;
                if(! cThis->ReportData(&temp,(void *)&(strResult),strResult.length()) ) {
					PING_DEBUG_S(Report data failed!);
				}
				else {
					iter_temp->bChange = false; // 重置
                    PING_DEBUG_V("Report data: " + strResult);
				}
			}
        }		
		if(cThis->m_pthreadnum_alive == 0 ) {
			break; //ping处理线程池全部退出后则退出该线程
		}
		if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
    cThis->m_bIsRunning = false;
    cThis->m_vecThreadPool.clear();
 	PING_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}

void* CPingMgr::OnPingHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CPingMgr *cThis = reinterpret_cast<CPingMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int timout	   = 1000000;  // ping的超时时间(单位:microsecond)
	int nPolocyNum = cThis->m_vecPolicy.size();

    while( cThis->m_vecThreadPool[thread_num].b_state ) {
        /*
		if(cThis->m_bIsRunning ) {		
			bd_ping bdPing;
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
                int timecost = 0;  // 微秒(usec)
				bool bConnect = bdPing.ping(cThis->m_vecPolicy[i].strIP, timout, &timecost);
				int nRes = bConnect?2:1; //1:不通 2:连通
				//修改ping结果变化状态
				if (cThis->m_vecPolicy[i].nResult != nRes) {
					cThis->m_vecPolicy[i].nResult = nRes;
                    cThis->m_vecPolicy[i].nTimecost = timecost;
					cThis->m_vecPolicy[i].bChange = true; // 需要发送结果给分析引擎
				}
			}
            sleep(m_pingConfig.nSleepsec); // ping的频率
        }*/
        if(cThis->m_bIsRunning ) {
            bd_ping bdPing;
            for(int i = thread_num; i < nPolocyNum; i += cThis->m_pthreadnum)
            {
                int timecost = 0;
                bool bConnect = false;
                int nRes = 1; //1:不通 2:连通

                string strtmp = cThis->m_vecPolicy[i].strIP;
                int lenth = strtmp.length();

                if(strtmp[lenth - 1] == '0' && strtmp[lenth - 2] == '.') //判断是否为网段(以.0结尾)
                {
                    nRes = 2;
                } else
                {
                    bConnect = bdPing.ping(cThis->m_vecPolicy[i].strIP, timout, &timecost);
                    nRes = bConnect? 2:1; //1:不通 2:连通
                }

                //修改ping结果变化状态
                if (cThis->m_vecPolicy[i].nResult != nRes) {
                    cThis->m_vecPolicy[i].nResult = nRes;
                    cThis->m_vecPolicy[i].bChange = true; // 需要发送结果给分析引擎
                }

                usleep(70000); //休眠70 ms
            }

            for(int i = 0; i < cThis->m_pingConfig.nSleepsec/5; i++) // ping的频率
            {
                sleep(5);
                if(!cThis->m_vecThreadPool[thread_num].b_state)
                    break;                                      //收到停止命令, 打断休眠
            }
        }
    }

	--cThis->m_pthreadnum_alive;
 	PING_INFO_S(OnPingHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CPingMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



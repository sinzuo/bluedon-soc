#include "BdSarMgr.h"
#include "utils/bdstring.h"
#include "task/bd_task.h"
#include <iostream>
#include <cstdio>
#include "bdsar.h"

using namespace Poco;
using namespace std;

string log(const char* format, ...) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, 4096, format, ap);
    va_end(ap);
    string str = buf;
    return str;
}

CBdSarMgr::CBdSarMgr(const string &strConfigName)
{
    m_strConfigName = strConfigName;
    memset(&m_BdSarConfig,0,sizeof(m_BdSarConfig));
    m_pFuncReportData  = NULL;
    m_pFuncFetchData   = NULL;
    m_bIsRunning       = false;
    m_pthreadnum       = 0;
    m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

    m_vecPolicy.clear();
    m_vecThreadPool.clear();
}

CBdSarMgr::~CBdSarMgr(void)
{
    m_vecPolicy.clear();
    m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CBdSarMgr::Init(void) {
    if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
    }

    if(!LoadConfig()) {
        return false;
    }

    if( strlen(m_BdSarConfig.chLog4File) > 0 ) {
        if(access(m_BdSarConfig.chLog4File, F_OK) != 0) {
            return false;
        } else {
            try { 
                log4cxx::PropertyConfigurator::configure(m_BdSarConfig.chLog4File);
                //g_logger = Logger::getLogger("LOG4_PROXY"); 
            } catch (Exception &) { 
                return false;
            }
        }
    } else {
        return false;
    }

    log_info(" CBdSarMgr::Init() successed...");

    printConfig();

    return true;
}

bool CBdSarMgr::IsRunning(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
    return m_bIsRunning;
}
UInt32 CBdSarMgr::GetModuleId(void) 
{
    return m_BdSarConfig.nModuleId;
}
UInt16 CBdSarMgr::GetModuleVersion(void) 
{
    return m_BdSarConfig.wModuleVersion;
}
string CBdSarMgr::GetModuleName(void) 
{
    return m_BdSarConfig.chModuleName;
}

//开始（下发)任务
bool CBdSarMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{   
    return true;
}

//停止（取消）任务
bool CBdSarMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;
}

bool CBdSarMgr::LoadConfig(void)
{   
    AutoPtr<IniFileConfiguration> pConf = NULL;
    try {
        pConf = new IniFileConfiguration(m_strConfigName);
    } catch (NotFoundException& exc ) {
        cerr << "BdSar LoadConfig IniFileConfiguration:" << exc.displayText() << endl;  
        return false;
    } catch (SyntaxException& exc) {
        cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;
    } catch (Exception& exc) {
        cerr << "BdSar LoadConfig:" << exc.displayText() << endl;   
        return false;
    }

    try {
        GET_CONF_ITEM_CHAR(log4configfile,m_BdSarConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_BdSarConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_BdSarConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_BdSarConfig.wModuleVersion);
        GET_CONF_ITEM_CHAR(record_separator,m_BdSarConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_BdSarConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(tasknum_perthread,m_BdSarConfig.nTaskNumPerThread);
        GET_CONF_ITEM_CHAR(submodule, m_BdSarConfig.sSubModule, 128);
        GET_CONF_ITEM_INT(interval, m_BdSarConfig.wInterval);

        if ( m_BdSarConfig.wInterval <= 0) {
            m_BdSarConfig.wInterval = 60;
        }

        if( m_BdSarConfig.nTaskNumPerThread <= 0 ) {
            m_BdSarConfig.nTaskNumPerThread = 5;  // 默认每个线程处理5个IP
        }
    }
    catch (NotFoundException& exc ) {
        cerr << "BdSar LoadConfig:" << exc.displayText() << endl;   
        return false;
    } catch (SyntaxException& exc) {
        cerr << "BdSar LoadConfig:" << exc.displayText() << endl;   
        return false;
    }
    return true;
}

void CBdSarMgr::printConfig() {
    log_info("<=========netservice configure information=========>");
    log_info(" log4configfile=>    [%s]", m_BdSarConfig.chLog4File);
    log_info(" moduleid=>          [%d]", m_BdSarConfig.nModuleId);
    log_info(" modulename=>        [%s]", m_BdSarConfig.chModuleName);
    log_info(" moduleversion=>     [%d]", m_BdSarConfig.wModuleVersion);
    log_info(" record_separator=>  [%s]", m_BdSarConfig.chRecordSep);
    log_info(" field_separator=>   [%s]", m_BdSarConfig.chFieldSep);
    log_info(" tasknum_perthread=> [%d]", m_BdSarConfig.nTaskNumPerThread);
    log_info(" sSubModule=>        [%s]", m_BdSarConfig.sSubModule);
    log_info("<=========netservice configure information=========>");
}

bool CBdSarMgr::Load(void)
{
    return true;
}

//调用方下发数据
bool CBdSarMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
    m_vecPolicy.clear();
    vector<string> vec_task_unit;
    string strRsep = m_BdSarConfig.chRecordSep;
    string_split(strPolicy,strRsep,vec_task_unit);

    log_info("<=======BdSar PlOCY=======>");

    int nNum = vec_task_unit.size();  // 策略数量
    for( int i=0; i<nNum; i++) {
        log_info(" BdSar POLOCY =>%s", vec_task_unit[i].c_str());
        tag_BdSar_policy_t tag_policy;
        vector<string> vec_temp;
        string strFsep = m_BdSarConfig.chFieldSep;
        // ip~port~module~cmd~authinfo
        string_split(vec_task_unit[i], strFsep, vec_temp, false);

        if (vec_temp.size() != 5) {
            log_error("Invalid policy string...");
            return false;
        }
        else {
            tag_policy.vec_options = vec_temp;
        }
        tag_policy.m_Finished = false;
        tag_policy.m_Result.clear();
        m_vecPolicy.push_back(tag_policy);
    }

    log_info("<=======BdSar PlOCY=======>");

    if (m_BdSarConfig.nTaskNumPerThread <= 0) {
        m_BdSarConfig.nTaskNumPerThread = 1;
    }
    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_BdSarConfig.nTaskNumPerThread);
    m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间

    return true;
}
//调用方获取数据
void * CBdSarMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//调用方释放获取到的内存
void CBdSarMgr::FreeData(void * pData)
{
    return;
}

//set callback for report function
void CBdSarMgr::SetReportData(pFunc_ReportData pCbReport)
{
    m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CBdSarMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
    m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CBdSarMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_BdSarConfig.nModuleId,pDataType,pData,dwLength);
    }
     return true;
}
//Fetch Data
const char* CBdSarMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//启动模块
bool CBdSarMgr::Start(void) {
    FastMutex::ScopedLock lock(m_ModMutex);

    //string str = "172.16.2.144~BDSEC~admin~888888~https://172.16.2.144:441/cgi-bin/admin/bdsec_daemon";
    //string str = "127.0.0.1~80~cpu~~|127.0.0.1~80~mem~~";
    //str += "|127.0.0.1~80~tcp|127.0.0.1~80~tcpx";
    //str += "|127.0.0.1~80~swap|127.0.0.1~80~udp";
    //str += "|172.16.12.48~80~apache~/Server-status~";
    //str += "|172.16.12.48~3128~squid~~";
    //str += "|172.16.12.48~8081~haproxy~/haproxy-stats~admin:123456";
    //str += "|172.16.12.48~80~nginx~/nginx-status~admin:123456";
    //SetData(NULL,&str,str.length());

    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        for (int i=0; i<m_pthreadnum; i++) {
            tag_BdSar_threadpool_t temp;
            memset(&temp, 0, sizeof(temp));
            temp.t_num = i;
            m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnBdSarHandle, this) != 0) {
                log_error("Create OnBdSarHandle Thread failed!");
                return false;
            }
            usleep(10000); // 10毫秒，启动线程需要一些执行时间
            m_pthreadnum_alive++;
            log_info("Create OnBdSarHandle Thread successed!");
        }
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            log_info("Create OnReportHandle Thread failed!");
            return false;
        }
        usleep(1000); // 1毫秒 
        log_info("Create OnReportHandle Thread successed!");
    }

	m_bIsLoaded = true;
    log_info("CBdSarMgr::Start successed!");
    return true;
}

//停止模块
bool CBdSarMgr::Stop(void) {
    FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

    for(int i=0; i< m_pthreadnum; i++) {
        m_vecThreadPool[i].b_state = false;
    }
    log_info("CBDSarMgr::Stop successed!");

    return true;
}

void *CBdSarMgr::OnFetchHandle(void *arg) {
    return NULL;
}

// 上报任务处理结果
void *CBdSarMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CBdSarMgr *cThis = reinterpret_cast<CBdSarMgr*>(arg);

    while(true) {

        if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
            cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
        }
        vector<tag_BdSar_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
            modintf_datatype_t temp;
            temp.nDataType     = MODULE_DATA_PERFORMANCE;
            temp.nMainCategory = MODULE_DATA_PERFORMANCE_BDSAR; 
            //int nChildTaskNum = iter_temp->m_mapFinished.size();
            //for(int i=0 ; i < nChildTaskNum; i++) {
                if( iter_temp->m_Finished ) { // 任务完成 
                    if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_Result),iter_temp->m_Result.length()) ) {
                        log_debug("Report module:%s data:%s failed!", iter_temp->vec_options[2].c_str(), iter_temp->m_Result.c_str());
                    } else {
                        log_debug("Report module:%s data:%s success!", iter_temp->vec_options[2].c_str(), iter_temp->m_Result.c_str());
                    }
                    iter_temp->m_Finished = false;  // 成功或失败下次都重新执行任务
                }
            //}
        }
        if(cThis->m_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程   
        if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
    cThis->m_bIsRunning = false;
	cThis->m_vecThreadPool.clear();
    log_info("OnReportHandle Thread exit...");

    return NULL;
}

//采集数据
void* CBdSarMgr::OnBdSarHandle(void *arg) {
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CBdSarMgr *cThis = reinterpret_cast<CBdSarMgr*>(arg);

    int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
    cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
    int nPolocyNum = cThis->m_vecPolicy.size();

    time_t lastTime = 0;        //上次采集时间

    //注册当前线程采集模块
    vector<bdSarBase*> cSubModule;
    std::vector<std::string> subModule;
    string sSubModule(cThis->m_BdSarConfig.sSubModule);
    string sFieldSep(cThis->m_BdSarConfig.chFieldSep);
    string_split(sSubModule, sFieldSep, subModule, 1);
    for (int i=thread_num; i < nPolocyNum; i += cThis->m_pthreadnum) {

        vector<string>::iterator it = subModule.begin();
        log_debug("check module:%s", cThis->m_vecPolicy[i].vec_options[2].c_str());

        for (;it != subModule.end(); it++) {

            if (*it != cThis->m_vecPolicy[i].vec_options[2]) {
                continue;
            }
            bdSarBase* pbdSar = bdSarFactory::create(it->c_str());

            if (pbdSar != NULL) {

                log_debug("register module:%s success.", it->c_str());
                log_debug("%s", cThis->m_vecPolicy[i].vec_options[0].c_str());
                pbdSar->setIP(cThis->m_vecPolicy[i].vec_options[0]);
                pbdSar->setPort(atoi(cThis->m_vecPolicy[i].vec_options[1].c_str()));
                string authinfo = toBase64(cThis->m_vecPolicy[i].vec_options[4].c_str());
                pbdSar->setAuthInfo(authinfo.c_str());
                pbdSar->setCmd(cThis->m_vecPolicy[i].vec_options[3].c_str());
                pbdSar->setItem(i);
                //pbdSar->setInterval(60);
                cSubModule.push_back(pbdSar);

            }
            else {

                log_warn("register module:%s failed.", it->c_str());
            }
            break;
        }
    }

    while( cThis->m_vecThreadPool[thread_num].b_state) {
        if(cThis->m_bIsRunning) {
            if (time(NULL) - lastTime < 60) {
                sleep(1);
                continue;
            }


            log_info("collect clock!");

            log_info("thread_num:%d, nPolocyNum:%d, cThis->m_pthreadnum:%d", thread_num, nPolocyNum, cThis->m_pthreadnum);

            vector<bdSarBase*>::iterator cIt = cSubModule.begin();
            string buf;

            for (;cIt != cSubModule.end(); cIt++) {

                if (lastTime == 0) {
                    //(*cIt)->setInterval(cThis->m_BdSarConfig.wInterval);
                    break;
                }
                else {
                    (*cIt)->setInterval(time(NULL) - lastTime);
                }

                log_info("time:%lu, lastTime:%lu", time(NULL), lastTime);

                int ret = (*cIt)->collect();
                if (ret != 0) {
                    log_warn("module:[%s] collect failed, please check!", (*cIt)->getModName().c_str());
                    continue;
                }
                log_debug("%s, %s, collect data success!", (*cIt)->getIP().c_str(),(*cIt)->getModName().c_str());

                buf = (*cIt)->Output();

                log_debug("%s, %s, Output data success!", (*cIt)->getIP().c_str(),(*cIt)->getModName().c_str());
                if (buf.empty()) {
                    log_warn("%s is empty!",(*cIt)->getModName().c_str());
                    continue;
                }

                string prefix = (*cIt)->getIP() + "~";
                prefix += toModuleNo((*cIt)->getModName()) + "|";
                cThis->m_vecPolicy[(*cIt)->getItem()].m_Result = prefix + buf;
                cThis->m_vecPolicy[(*cIt)->getItem()].m_Finished = true;
            }

            lastTime = time(NULL);
            usleep(300);
        }
    }
    --cThis->m_pthreadnum_alive;
    log_info("OnBdSarHandle Thread exit...");

    return NULL;
}

//Base64数据加密
string CBdSarMgr::toBase64(string source) {
    stringstream in;
    Base64Encoder b64in(in);
    b64in << source.c_str();
    b64in.close();
    return in.str();
}

string CBdSarMgr::toModuleNo(string module) {
    if (module == "nginx") {
        return "1";
    }
    else if (module == "apache") {
        return "2";
    }
    else if (module == "haproxy") {
        return "3";
    }
    else if (module == "squid") {
        return "4";
    }
    else {
        return "0";
    }
}

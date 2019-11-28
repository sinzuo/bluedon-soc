
#include "ControllerMgr.h"
#include "utils/bdstring.h"
#include "task/bd_task.h"
#include "ssh/bd_ssh_control.h"
#include <iostream>
#include <cstdio>
#include <vector>

using namespace Poco;
using namespace std;


CController::CController(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_controllerConfig,0,sizeof(m_controllerConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CController::~CController(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CController::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_controllerConfig.chLog4File) > 0 ) {
	    if(access(m_controllerConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_controllerConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    CONTROLLER_INFO_S( CController::Init() successed...);
	printConfig();

    return true;	
}

bool CController::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CController::GetModuleId(void) 
{
	return m_controllerConfig.nModuleId;
}
UInt16 CController::GetModuleVersion(void) 
{
	return m_controllerConfig.wModuleVersion;
}
string CController::GetModuleName(void) 
{
	return m_controllerConfig.chModuleName;
}

//开始（下发)任务
bool CController::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CController::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CController::LoadConfig(void)
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
		GET_CONF_ITEM_CHAR(log4configfile,m_controllerConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_controllerConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_controllerConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_controllerConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_controllerConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_controllerConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_controllerConfig.nTaskNumPerThread);
		if( m_controllerConfig.nTaskNumPerThread <= 0 ) {
			m_controllerConfig.nTaskNumPerThread = 5;  // 默认每个线程处理5个IP
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

void CController::printConfig() {
	CONTROLLER_INFO_S(<=========netservice configure information=========>);
	CONTROLLER_INFO_V(" log4configfile=>    ["+string(m_controllerConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_controllerConfig.nModuleId);
    CONTROLLER_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	CONTROLLER_INFO_V(" modulename=>        ["+string(m_controllerConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_controllerConfig.wModuleVersion);
	CONTROLLER_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	CONTROLLER_INFO_V(" record_separator=>  ["+string(m_controllerConfig.chRecordSep)+string("]"));
	CONTROLLER_INFO_V(" field_separator=>   ["+string(m_controllerConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_controllerConfig.nTaskNumPerThread);
	CONTROLLER_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
	CONTROLLER_INFO_S(<=========netservice configure information=========>);
}

bool CController::Load(void)
{
	return true;
}

//调用方下发数据
bool CController::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
    printf("recv strPolicy:%s",strPolicy.c_str());
	m_vecPolicy.clear();	
	tag_controller_policy_t tag_policy;
    vector<string> vec_policy;
	string strFsep = m_controllerConfig.chFieldSep;
    string strRsep = m_controllerConfig.chRecordSep;
    //标识ID~协议ID(协议ID 1.SNMP 2.SSH 3.TELNET 4.BDSEC)~IP1~COMMAND~请求参数串(用户名~密码~URL~param)|...
    string_split(strPolicy,strRsep,vec_policy);

    CONTROLLER_INFO_S(<=======CONTROLLER PlOCY=======>);
    for(int i = 0; i < vec_policy.size(); i++)
    {
        vector<string> vec_temp;
        string_split(vec_policy[i], strFsep, vec_temp);
        tag_policy.vec_options = vec_temp;

        CONTROLLER_INFO_V(vec_policy[i]);
        if(vec_temp.size() < 7)
        {
            CONTROLLER_ERROR_S(Policy is Unavailable...);
            continue;
        }
        m_vecPolicy.push_back(tag_policy);
    }
    CONTROLLER_INFO_S(<=======CONTROLLER PlOCY=======>);

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_controllerConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}

//调用方获取数据
void * CController::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CController::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CController::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CController::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CController::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_controllerConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CController::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CController::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
	//string str = "172.16.2.144~BDSEC~admin~888888~https://172.16.2.144:441/cgi-bin/admin/bdsec_daemon";
	//SetData(NULL,&str,str.length());
    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        m_vecThreadPool.clear();
        for (int i=0; i < m_pthreadnum; i++) {
			tag_controller_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnControllerHandle, this) != 0) {
				CONTROLLER_ERROR_S(Create OnControllerHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
            CONTROLLER_INFO_S(Create OnControllerHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			CONTROLLER_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		CONTROLLER_INFO_S(Create OnReportHandle Thread successed!);	
	}	

	m_bIsLoaded = false;
	CONTROLLER_INFO_S(CController::Start successed!);
	return true;
}

//停止模块
bool CController::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}
	CONTROLLER_INFO_S(CController::Stop successed!);
    return true;
}

void *CController::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CController::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CController *cThis = reinterpret_cast<CController*>(arg);

    if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
	while(true) {
        vector<tag_controller_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_CONTROLLER;	
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i < nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成	
					if(!cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
                        CONTROLLER_ERROR_S(Report data failed!);
					} else {
						CONTROLLER_DEBUG_S(Report data success!);
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
 	CONTROLLER_INFO_S(OnReportHandle Thread exit...);
    return NULL;

}

void* CController::OnControllerHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CController *cThis = reinterpret_cast<CController*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
    string result = "";
	while( cThis->m_vecThreadPool[thread_num].b_state) {
        if(cThis->m_bIsRunning) {
            for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
                if(cThis->m_vecPolicy[i].m_mapFinished[0] == false)
                {
                    vector<string>  v_options_tmp = cThis->m_vecPolicy[i].vec_options;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除标识结点
                    vector<string>::iterator iter = v_options_tmp.begin();
                    string protocol = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除协议ID结点
                    iter = v_options_tmp.begin();
                    string ip = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除ip结点
                    iter = v_options_tmp.begin();
                    string command = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除command结点

                    if(protocol.compare("4") == 0) //bdsec协议
                    {
                        CTask Task(v_options_tmp);
                        vector<string (CTask::*)()> vecFunctions;
                        vecFunctions.push_back(&CTask::commond_shutdown);        //shutdown
                        vecFunctions.push_back(&CTask::commond_reboot);          //reboot
                        vecFunctions.push_back(&CTask::commond_restart_service); //restartservice
                        vecFunctions.push_back(&CTask::commond_stop_service);    //stopservice
                        vecFunctions.push_back(&CTask::commond_syntime);         //syntime
                        vecFunctions.push_back(&CTask::commond_factory_recover); //factoryrecover

                        int nCmd = atoi(command.c_str());
                        if(nCmd > 0 && nCmd <= 6)
                        {
                            result = (Task.*vecFunctions[nCmd - 1])();
                        } else
                        {
                            result = "1";
                            CONTROLLER_ERROR_V("Unknow bdsec command...");
                        }
                    }
                    else if(protocol.compare("2") == 0) //ssh协议
                    {
                        CONTROLLER_INFO_V("Begin ssh ...");
                        if(v_options_tmp.size() < 4)
                            v_options_tmp.push_back(string(""));
                        CONTROLLER_DEBUG_V(v_options_tmp[2] + ", " + v_options_tmp[3]);

                        bd_ssh_control sshc(v_options_tmp[2], v_options_tmp[3]); //, v_options_tmp[3]

                        try{
                            sshc.authentication(v_options_tmp[0], v_options_tmp[1]);
                        }
                        catch(...){
                            result = "login_err";
                            CONTROLLER_ERROR_V("ssh Login error...");
                            goto REPORT;
                        }

                        if(command.compare("1") == 0) //shutdown
                        {
                            result = sshc.shutdown();
                        }

                        if(command.compare("2") == 0) //reboot
                        {
                            result = sshc.reboot();
                        }

                        if(command.compare("3") == 0) //restartservice
                        {
                            result = sshc.restartservice();
                        }

                        if(command.compare("4") == 0) //stopservice
                        {
                            result = sshc.stopservice();
                        }

                        if(command.compare("5") == 0) //syntime
                        {
                            result = sshc.syntime();
                        }
                    }
                    else {
                        result = "1";
                        CONTROLLER_ERROR_V("Unknow protocol...");
                    }

                REPORT:
                    //ip~结果字符串
                    string SrcReport = cThis->m_vecPolicy[i].vec_options[0];
                    SrcReport += cThis->m_controllerConfig.chFieldSep;
                    SrcReport += cThis->m_vecPolicy[i].vec_options[1];
                    SrcReport += cThis->m_controllerConfig.chFieldSep;
                    SrcReport += cThis->m_vecPolicy[i].vec_options[2];
                    SrcReport += cThis->m_controllerConfig.chFieldSep;
                    SrcReport += cThis->m_vecPolicy[i].vec_options[3];
                    SrcReport += cThis->m_controllerConfig.chFieldSep;
                    SrcReport += result;
                    cThis->m_vecPolicy[i].m_mapResult[0] = SrcReport;
                    cThis->m_vecPolicy[i].m_mapFinished[0] = true;
                    CONTROLLER_DEBUG_V("controller result:"+SrcReport);
                    sleep(3);
                }//end if(cThis->m_vecPolicy[i].m_mapFinished[0] == false)
            }
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
    CONTROLLER_INFO_S(OnControllerHandle Thread exit...);
    return NULL;
}

//base64解码
string CController::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



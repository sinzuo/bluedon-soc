
#include "MonitorMgr.h"
#include "utils/bdstring.h"
#include "task/bd_task.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CMonitorMgr::CMonitorMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_monitorConfig,0,sizeof(m_monitorConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CMonitorMgr::~CMonitorMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CMonitorMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_monitorConfig.chLog4File) > 0 ) {
	    if(access(m_monitorConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_monitorConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    MONITOR_INFO_S( CMonitorMgr::Init() successed...);
	printConfig();

    return true;	
}

bool CMonitorMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CMonitorMgr::GetModuleId(void) 
{
	return m_monitorConfig.nModuleId;
}
UInt16 CMonitorMgr::GetModuleVersion(void) 
{
	return m_monitorConfig.wModuleVersion;
}
string CMonitorMgr::GetModuleName(void) 
{
	return m_monitorConfig.chModuleName;
}

//开始（下发)任务
bool CMonitorMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CMonitorMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CMonitorMgr::LoadConfig(void)
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
		GET_CONF_ITEM_CHAR(log4configfile,m_monitorConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_monitorConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_monitorConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_monitorConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_monitorConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_monitorConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_monitorConfig.nTaskNumPerThread);
        GET_CONF_ITEM_INT(nSleepsec,m_monitorConfig.nSleepsec);
		if( m_monitorConfig.nTaskNumPerThread <= 0 ) {
            m_monitorConfig.nTaskNumPerThread = 100;  // 默认每个线程处理5个IP
		}
        if( m_monitorConfig.nSleepsec <= 0 ) {
            m_monitorConfig.nSleepsec = 60;  // 默认睡眠60秒
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

void CMonitorMgr::printConfig() {
	MONITOR_INFO_S(<=========netservice configure information=========>);
	MONITOR_INFO_V(" log4configfile=>    ["+string(m_monitorConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_monitorConfig.nModuleId);
    MONITOR_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	MONITOR_INFO_V(" modulename=>        ["+string(m_monitorConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_monitorConfig.wModuleVersion);
	MONITOR_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	MONITOR_INFO_V(" record_separator=>  ["+string(m_monitorConfig.chRecordSep)+string("]"));
	MONITOR_INFO_V(" field_separator=>   ["+string(m_monitorConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_monitorConfig.nTaskNumPerThread);
	MONITOR_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    sprintf(chTemp,"%d",m_monitorConfig.nSleepsec);
    MONITOR_INFO_V(" nSleepsec=> ["+string(chTemp)+string("]"));
	MONITOR_INFO_S(<=========netservice configure information=========>);
}

bool CMonitorMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CMonitorMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
	vector<string> vec_task_unit;
	string strRsep = m_monitorConfig.chRecordSep;
	string_split(strPolicy,strRsep,vec_task_unit);
	
	MONITOR_INFO_S(<=======MONITOR PlOCY=======>);
	int nNum = vec_task_unit.size();  // 监控IP数量
    for( int i=0; i<nNum; i++) {
		MONITOR_INFO_V(" MONITOR POlOCY =>" + vec_task_unit[i]);		
		tag_monitor_policy_t tag_policy;
		vector<string> vec_temp;
		string strFsep = m_monitorConfig.chFieldSep;
		// IP~请求参数串(BDSEC~用户名~密码~URL)
		string_split(vec_task_unit[i],strFsep,vec_temp);
		
		if( vec_temp[1] == "BDSEC" && vec_temp.size() != 5) {
			MONITOR_ERROR_S(Invalid polocy string...);
			return false;
        } else if(vec_temp[1] == "SNMP" && vec_temp.size() != 11)
		{
            MONITOR_ERROR_S(Invalid polocy string...);
		   return false;
		}
        else{
            tag_policy.vec_options = vec_temp;
           }
        m_vecPolicy.push_back(tag_policy);
    }
	MONITOR_INFO_S(<=======MONITOR PlOCY=======>);	
	
    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_monitorConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}
//调用方获取数据
void * CMonitorMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CMonitorMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CMonitorMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CMonitorMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CMonitorMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_monitorConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CMonitorMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CMonitorMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
	//string str = "172.16.2.144~BDSEC~admin~888888~https://172.16.2.144:441/cgi-bin/admin/bdsec_daemon";
	//SetData(NULL,&str,str.length());

    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
			tag_monitor_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnMonitorHandle, this) != 0) {
				MONITOR_ERROR_S(Create OnMonitorHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
			MONITOR_INFO_S(Create OnMonitorHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			MONITOR_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		MONITOR_INFO_S(Create OnReportHandle Thread successed!);	
	}	

	m_bIsLoaded = true;
	MONITOR_INFO_S(CMonitorMgr::Start successed!);
	return true;
}

//停止模块
bool CMonitorMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}	
	MONITOR_INFO_S(CMonitorMgr::Stop successed!);
    return true;
}

void *CMonitorMgr::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CMonitorMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CMonitorMgr *cThis = reinterpret_cast<CMonitorMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_monitor_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_MONITOR;	
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i < nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成	
					if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
						MONITOR_DEBUG_S(Report data failed!);
					} else {
						MONITOR_DEBUG_S(Report data success!);
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
 	MONITOR_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}

void* CMonitorMgr::OnMonitorHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CMonitorMgr *cThis = reinterpret_cast<CMonitorMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {		
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
				 
                vector<string> v_options_tmp = cThis->m_vecPolicy[i].vec_options;
				v_options_tmp.erase(v_options_tmp.begin()); // 使参数串符合CTask输入格式
				CTask MonitorTask(v_options_tmp);		 

				if ( !cThis->m_vecPolicy[i].m_mapFinished[0] ) { // cpu		
				    MONITOR_INFO_V(string("Ready to get cpu rate from ") + cThis->m_vecPolicy[i].vec_options[0]);
                    int nCpu = 0;
					char chCpu[4] = {0};					
                    if ( !MonitorTask.getCpuUsage(nCpu)) {
						MONITOR_ERROR_S(Get CPU rate failed!);
                    } else {
       					sprintf(chCpu,"%d",nCpu);	
                    }
					// IP~1|CPU使用率
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "1";
					strResult += cThis->m_monitorConfig.chRecordSep;
					strResult += chCpu; 
					
					cThis->m_vecPolicy[i].m_mapResult[0] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[0] = true;
					
				}

				if ( !cThis->m_vecPolicy[i].m_mapFinished[1] ) { // memory
				    MONITOR_INFO_V(string("Ready to get memory rate from ") + cThis->m_vecPolicy[i].vec_options[0]);
                    int nMem = 0;
                    char chMem[4] = {0};

                    if ( !MonitorTask.getMemUsage(nMem)) {
                        MONITOR_ERROR_S(Get MEMORY rate failed!);
                    } else {
                        sprintf(chMem,"%d",nMem);
                    }
//                    sprintf(chMem, "%d", nMem);
					// IP~2|内存使用率
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "2";
					strResult += cThis->m_monitorConfig.chRecordSep;
					strResult += chMem; 
					
					cThis->m_vecPolicy[i].m_mapResult[1] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[1] = true;
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[2] ) { // disk
				    MONITOR_INFO_V(string("Ready to get disk rate from ") + cThis->m_vecPolicy[i].vec_options[0]);
                    map <string,string> mapDiskUsage;
					if ( !MonitorTask.getDiskUsage(mapDiskUsage)) {
						MONITOR_ERROR_S(Get DISK rate failed!);
                    }
					
					// IP~3|磁盘1名称~磁盘1使用率|...
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "3";
					strResult += cThis->m_monitorConfig.chRecordSep;
					
                    map <string,string>::iterator iter_map = mapDiskUsage.begin();
					for(; iter_map != mapDiskUsage.end(); ) {
                        strResult += iter_map->first;
						strResult += cThis->m_monitorConfig.chFieldSep;
						strResult += iter_map->second;
						
						if(++iter_map != mapDiskUsage.end()) {
							strResult += cThis->m_monitorConfig.chRecordSep;
						}
					}
					
					cThis->m_vecPolicy[i].m_mapResult[2] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[2] = true;					
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[3] ) { // service state
				    MONITOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
					map <string,string>  mapServiceState;
					if ( !MonitorTask.getServiceState(mapServiceState)) {
						MONITOR_ERROR_S(Get Service state failed!);
                    }
					
					// IP~4|service名称~状态|...
					string strResult = cThis->m_vecPolicy[i].vec_options[0];
					strResult += cThis->m_monitorConfig.chFieldSep;
					strResult += "4";
					strResult += cThis->m_monitorConfig.chRecordSep;
					
                    map <string,string>::iterator iter_map = mapServiceState.begin();
					for(; iter_map != mapServiceState.end(); ) {
                        strResult += iter_map->first;
						strResult += cThis->m_monitorConfig.chFieldSep;
						strResult += iter_map->second;

					    if(++iter_map != mapServiceState.end()) {
						    strResult += cThis->m_monitorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[3] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[3] = true;					
				}

               if(v_options_tmp[0].compare(PROTOCOL_SNMP) == 0){
                   //网关流量监信息
                   if ( !cThis->m_vecPolicy[i].m_mapFinished[4] ) { // service state
                       MONITOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
                       NetWorkInfoList NetWorkInfos;
                       char tmp[128];
                       memset(tmp,0,128);
                       if ( !MonitorTask.getNetworkUsage(NetWorkInfos)) {
                           MONITOR_ERROR_S(Get tNetwork Usage failed!);
                       }

                       // 网关名称1~网关mac地址~网关流量状态(up/down)~下行流量~上行流量|...
                       string strResult = cThis->m_vecPolicy[i].vec_options[0];
                       strResult += cThis->m_monitorConfig.chFieldSep;
                       strResult += "5";
                       strResult += cThis->m_monitorConfig.chRecordSep;

                       vector<NetworkInfo>::iterator iter = NetWorkInfos.begin();
                       for(; iter != NetWorkInfos.end(); ) {
                           strResult += iter->if_name;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           strResult += iter->if_mac;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           sprintf(tmp,"%d",iter->if_status);
                           strResult += tmp;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           sprintf(tmp,"%lld",iter->if_inOctet);
                           strResult += tmp;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           sprintf(tmp,"%lld",iter->if_outOctet);
                           strResult += tmp;
                           if(++iter != NetWorkInfos.end()) {
                               strResult += cThis->m_monitorConfig.chRecordSep;
                           }
                       }

                       cThis->m_vecPolicy[i].m_mapResult[4] = strResult;
                       cThis->m_vecPolicy[i].m_mapFinished[4] = true;
                   }
                   //安装列表信息
                   if ( !cThis->m_vecPolicy[i].m_mapFinished[5] ) { // service state
                       MONITOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
                       SWInstallInfoList  SWInstallInfos;
                       if ( !MonitorTask.getSWInstalledUsage(SWInstallInfos)) {
                           MONITOR_ERROR_S(Get SWInstalled Usage failed!);
                       }

                       //安装服务名称1~类型(unknown/operatingSystem/deviceDriver/application)~具体信息|...
                       string strResult = cThis->m_vecPolicy[i].vec_options[0];
                       strResult += cThis->m_monitorConfig.chFieldSep;
                       strResult += "6";
                       strResult += cThis->m_monitorConfig.chRecordSep;

                       vector<SWInstallInfo>::iterator iter = SWInstallInfos.begin();
                       for(; iter != SWInstallInfos.end(); ) {
                           strResult += iter->sw_name;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           string type = "";
                           switch(iter->sw_type)
                           {
                           case 1:
                               type = "unknown";
                               break;
                           case 2:
                               type = "operatingSystem";
                               break;
                           case 3:
                               type = "deviceDriver";
                               break;
                           case 4:
                               type = "application";
                               break;
                           default:
                               break;
                           }
                           strResult += type;
                           strResult += cThis->m_monitorConfig.chFieldSep;
                           strResult += iter->sw_date;
                           if(++iter != SWInstallInfos.end()) {
                               strResult += cThis->m_monitorConfig.chRecordSep;
                           }
                       }

                       cThis->m_vecPolicy[i].m_mapResult[5] = strResult;
                       cThis->m_vecPolicy[i].m_mapFinished[5] = true;
                   }
                   //系统信息
                   if ( !cThis->m_vecPolicy[i].m_mapFinished[6] ) {
                       MONITOR_INFO_V(string("Ready to get system information from ") + cThis->m_vecPolicy[i].vec_options[0]);
                       string systemInfo;
                       if ( !MonitorTask.getSysInfo(systemInfo)) {
                           MONITOR_ERROR_S(Get System information failed!);
                       }

                       //ip~7|操作系统~主机名
                       string strResult = cThis->m_vecPolicy[i].vec_options[0];
                       strResult += cThis->m_monitorConfig.chFieldSep;
                       strResult += "7";
                       strResult += cThis->m_monitorConfig.chRecordSep;
                       strResult += systemInfo;

                       cThis->m_vecPolicy[i].m_mapResult[6] = strResult;
                       cThis->m_vecPolicy[i].m_mapFinished[6] = true;
                   }
               } //end PROTOCOL_SNMP

               if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                   break;
            }            
            for(int i = 0; i < cThis->m_monitorConfig.nSleepsec/5; i++)
            {
                sleep(5);
                if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                    break;
            }
		}
	}
	--cThis->m_pthreadnum_alive;
 	MONITOR_INFO_S(OnMonitorHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CMonitorMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}




#include "InspectorMgr.h"
#include "utils/bdstring.h"
#include "task/bd_task.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CInspectorMgr::CInspectorMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_inspectorConfig,0,sizeof(m_inspectorConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CInspectorMgr::~CInspectorMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CInspectorMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_inspectorConfig.chLog4File) > 0 ) {
	    if(access(m_inspectorConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_inspectorConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    INSPECTOR_INFO_S( CInspectorMgr::Init() successed...);
	printConfig();

    return true;	
}

bool CInspectorMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CInspectorMgr::GetModuleId(void) 
{
	return m_inspectorConfig.nModuleId;
}
UInt16 CInspectorMgr::GetModuleVersion(void) 
{
	return m_inspectorConfig.wModuleVersion;
}
string CInspectorMgr::GetModuleName(void) 
{
	return m_inspectorConfig.chModuleName;
}

//开始（下发)任务
bool CInspectorMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CInspectorMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CInspectorMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "INSPECTOR LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "INSPECTOR LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_inspectorConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_inspectorConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_inspectorConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_inspectorConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_inspectorConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_inspectorConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_inspectorConfig.nTaskNumPerThread);
		if( m_inspectorConfig.nTaskNumPerThread <= 0 ) {
			m_inspectorConfig.nTaskNumPerThread = 1;  // 默认每个线程处理1个任务
		}
	}
	catch (NotFoundException& exc ) {
		cerr << "INSPECTOR LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "INSPECTOR LoadConfig:" << exc.displayText() << endl;	
		return false;
	}
    return true;
}

void CInspectorMgr::printConfig() {
	INSPECTOR_INFO_S(<=========netservice configure information=========>);
	INSPECTOR_INFO_V(" log4configfile=>    ["+string(m_inspectorConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_inspectorConfig.nModuleId);
    INSPECTOR_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	INSPECTOR_INFO_V(" modulename=>        ["+string(m_inspectorConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_inspectorConfig.wModuleVersion);
	INSPECTOR_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	INSPECTOR_INFO_V(" record_separator=>  ["+string(m_inspectorConfig.chRecordSep)+string("]"));
	INSPECTOR_INFO_V(" field_separator=>   ["+string(m_inspectorConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_inspectorConfig.nTaskNumPerThread);
	INSPECTOR_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
	INSPECTOR_INFO_S(<=========netservice configure information=========>);
}

bool CInspectorMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CInspectorMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
	vector<string> vec_task_unit;
	string strRsep = m_inspectorConfig.chRecordSep;
	string_split(strPolicy,strRsep,vec_task_unit,false);
	
	INSPECTOR_INFO_S(<=======INSPECTOR PlOCY=======>);
	INSPECTOR_INFO_V(" INSPECTOR POlOCY =>" + strPolicy);
	INSPECTOR_INFO_S(<=======INSPECTOR PlOCY=======>);	
	
	int nTaskNum = atoi(vec_task_unit[0].c_str());
	if( vec_task_unit.size() != (nTaskNum * 4 + 1)) {
		INSPECTOR_ERROR_S(Invalid polocy string...);
		return false;
	}
    for( int i=0; i<nTaskNum; i++) {
		tag_inspector_policy_t tag_policy;
		vector<string> vec_temp;
		string strFsep = m_inspectorConfig.chFieldSep;
		// 任务ID~任务类型(1:一次性任务)
		string_split(vec_task_unit[i*4 + 1],strFsep,vec_temp,false);
		if( vec_temp.size() != 2) {
			INSPECTOR_ERROR_S(Invalid polocy string...);
			return false;
		} else {
			tag_policy.strTaskId = vec_temp[0];
			if( atoi(vec_temp[1].c_str()) == 1) tag_policy.bOnce = true; // 一次性任务
		}
		// 分~时~日~月~周
		tag_policy.strTimePolocy = vec_task_unit[i*4 + 2];
		// IP
		tag_policy.strIP = vec_task_unit[i*4 + 3];
		// 协议类型~参数字符串(参数随协议类型不同而不同) 
		string_split(vec_task_unit[i*4 + 4],strFsep,tag_policy.vec_options);
		if(tag_policy.vec_options[0].compare(PROTOCOL_BDSEC)==0){  // BDSEC~用户~密码~bdsecURL  
            printf(":::::::::::::%d\n",tag_policy.vec_options.size());
            if(tag_policy.vec_options.size() != 4) return false;
		} else if(tag_policy.vec_options[0].compare(PROTOCOL_SNMP)==0)
		{
             printf(":::::::::::::%d\n",tag_policy.vec_options.size());
            if(tag_policy.vec_options.size() != 10)
            {
                INSPECTOR_ERROR_S(Invalid snmp options size...);
                return false;
            }
		}
		else{
            return false;
            }
        m_vecPolicy.push_back(tag_policy);
    }

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_inspectorConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}

//调用方获取数据
void * CInspectorMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CInspectorMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CInspectorMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CInspectorMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CInspectorMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_inspectorConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CInspectorMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CInspectorMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
	//string str = "1|888888~1|*~*~*~*~*|172.16.2.144|BDSEC~admin~888888~https://172.16.2.144:441/cgi-bin/admin/bdsec_daemon";
	//SetData(NULL,&str,str.length());

    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
			tag_inspector_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnInspectorHandle, this) != 0) {
                  INSPECTOR_ERROR_S(Create OnInspectorHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
			INSPECTOR_INFO_S(Create OnInspectorHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			INSPECTOR_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		INSPECTOR_INFO_S(Create OnReportHandle Thread successed!);	
	}	

	m_bIsLoaded = true;
	INSPECTOR_INFO_S(CInspectorMgr::Start successed!);
	return true;
}

//停止模块
bool CInspectorMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}	
	INSPECTOR_INFO_S(CInspectorMgr::Stop successed!);
    return true;
}

void *CInspectorMgr::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CInspectorMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CInspectorMgr *cThis = reinterpret_cast<CInspectorMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_inspector_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_INSPECTOR;	
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i<nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成才上报	
					if (iter_temp->bOnce && iter_temp->m_mapReported[i]) continue;	// 一次性任务而且已经上报过了	
					if ( iter_temp->m_mapResult[i].length() > 0 ){
						if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
							INSPECTOR_DEBUG_S(Report data failed!);
						} else {
							INSPECTOR_DEBUG_S(Report data success!);
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
 	INSPECTOR_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}

void* CInspectorMgr::OnInspectorHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CInspectorMgr *cThis = reinterpret_cast<CInspectorMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {	
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
               bool bReady = check_cron(cThis->m_vecPolicy[i].strTimePolocy);
			  if (!bReady) continue;
				 
				CTask InspectorTask(cThis->m_vecPolicy[i].vec_options);		 
                if ( !cThis->m_vecPolicy[i].m_mapFinished[0] ) { // ping  
					INSPECTOR_INFO_V(string("Ready to ping ") + cThis->m_vecPolicy[i].strIP);
					int timeout  = 30000;  // ping的超时时间(单位:microsecond)
					int timecost = 0;
					bool bConnect = InspectorTask.getPingState(cThis->m_vecPolicy[i].strIP, timeout, &timecost);
					string strPing = bConnect?"2":"1"; //1:不通 2:连通
					// 任务标识~0|IP连通性
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "0";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					strResult += strPing; 
					
					cThis->m_vecPolicy[i].m_mapResult[0] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[0] = true; //ping任务执行完成
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[1] ) { // cpu		
				    INSPECTOR_INFO_V(string("Ready to get cpu rate from ") + cThis->m_vecPolicy[i].strIP);
                    int nCpu = 0;
					char chCpu[4] = {0};					
                    if ( !InspectorTask.getCpuUsage(nCpu)) {
						INSPECTOR_ERROR_S(Get CPU rate failed!);
                    } else {
       					sprintf(chCpu,"%d",nCpu);	
                    }
					// 任务标识~1|CPU使用率
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "1";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					strResult += chCpu; 
					
					cThis->m_vecPolicy[i].m_mapResult[1] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[1] = true;
					
				}

				if ( !cThis->m_vecPolicy[i].m_mapFinished[2] ) { // memory
				    INSPECTOR_INFO_V(string("Ready to get memory rate from ") + cThis->m_vecPolicy[i].strIP);
                    int nMem = 0;
					char chMem[4] = {0};                    
                    if ( !InspectorTask.getMemUsage(nMem)) {
						INSPECTOR_ERROR_S(Get MEMORY rate failed!);
                    } else {
       					sprintf(chMem,"%d",nMem);	
                    }
					// 任务标识~2|内存使用率
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;					
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "2";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					strResult += chMem; 
					
					cThis->m_vecPolicy[i].m_mapResult[2] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[2] = true;
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[3] ) { // disk
				    INSPECTOR_INFO_V(string("Ready to get disk rate from ") + cThis->m_vecPolicy[i].strIP);
                    map <string,string> mapDiskUsage;
					if ( !InspectorTask.getDiskUsage(mapDiskUsage)) {
						INSPECTOR_ERROR_S(Get DISK rate failed!);
                    }
					
					// 任务标识~3|磁盘1名称~磁盘1使用率|...
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "3";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					
                    map <string,string>::iterator iter_map = mapDiskUsage.begin();
					for(; iter_map != mapDiskUsage.end(); ) {
                        strResult += iter_map->first;
						strResult += cThis->m_inspectorConfig.chFieldSep;
						strResult += iter_map->second;
						
						if(++iter_map != mapDiskUsage.end()) {
							strResult += cThis->m_inspectorConfig.chRecordSep;
						}
					}
					
					cThis->m_vecPolicy[i].m_mapResult[3] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[3] = true;					
				}
				
				if ( !cThis->m_vecPolicy[i].m_mapFinished[4] ) { // service state
				    INSPECTOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].strIP);
					map <string,string>  mapServiceState;
					if ( !InspectorTask.getServiceState(mapServiceState)) {
						INSPECTOR_ERROR_S(Get Service state failed!);
                    }
					
					// 任务标识~4|service名称~状态|...
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "4";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					
                    map <string,string>::iterator iter_map = mapServiceState.begin();
					for(; iter_map != mapServiceState.end(); ) {
                        strResult += iter_map->first;
						strResult += cThis->m_inspectorConfig.chFieldSep;
						strResult += iter_map->second;

					    if(++iter_map != mapServiceState.end()) {
						    strResult += cThis->m_inspectorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[4] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[4] = true;					
				}
                  //网关流量监信息
                if ( !cThis->m_vecPolicy[i].m_mapFinished[5] ) { // service state
				    INSPECTOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
					NetWorkInfoList NetWorkInfos;
                      char tmp[30];
                      memset(tmp,0,30);
					if ( !InspectorTask.getNetworkUsage(NetWorkInfos)) {
						INSPECTOR_ERROR_S(Get tNetwork Usage failed!);
                    }
					
					// 网关名称1~网关mac地址~网关流量状态(up/down)~下行流量~上行流量|...
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "5";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					
                      vector<NetworkInfo>::iterator iter = NetWorkInfos.begin();
					for(; iter != NetWorkInfos.end(); ) {
                           strResult += iter->if_name;
						strResult += cThis->m_inspectorConfig.chFieldSep;
						strResult += iter->if_mac;
                           strResult += cThis->m_inspectorConfig.chFieldSep;
                           sprintf(tmp,"%d",iter->if_status);
						strResult += tmp;
                           strResult += cThis->m_inspectorConfig.chFieldSep;
                           sprintf(tmp,"%d",iter->if_inOctet);
						strResult += tmp;
                           strResult += cThis->m_inspectorConfig.chFieldSep;
                           sprintf(tmp,"%d",iter->if_outOctet);
						strResult += tmp;
					    if(++iter != NetWorkInfos.end()) {
						    strResult += cThis->m_inspectorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[5] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[5] = true;					
				}
                //安装列表信息
                if ( !cThis->m_vecPolicy[i].m_mapFinished[6] ) { // service state
				    INSPECTOR_INFO_V(string("Ready to get service state from ") + cThis->m_vecPolicy[i].vec_options[0]);
					SWInstallInfoList  SWInstallInfos;
					if ( !InspectorTask.getSWInstalledUsage(SWInstallInfos)) {
						INSPECTOR_ERROR_S(Get SWInstalled Usage failed!);
                    }
					
					//安装服务名称1~类型(unknown/operatingSystem/deviceDriver/application)~具体信息|...
					string strResult = cThis->m_vecPolicy[i].strTaskId;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += cThis->m_vecPolicy[i].strIP;
					strResult += cThis->m_inspectorConfig.chFieldSep;
					strResult += "6";
					strResult += cThis->m_inspectorConfig.chRecordSep;
					
                      vector<SWInstallInfo>::iterator iter = SWInstallInfos.begin();
					for(; iter != SWInstallInfos.end(); ) {
                           strResult += iter->sw_name;
						strResult += cThis->m_inspectorConfig.chFieldSep;
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
                          strResult += cThis->m_inspectorConfig.chFieldSep;
                          strResult += iter->sw_date;
					    if(++iter != SWInstallInfos.end()) {
						    strResult += cThis->m_inspectorConfig.chRecordSep;
					    }
					}
					
					cThis->m_vecPolicy[i].m_mapResult[6] = strResult;
					cThis->m_vecPolicy[i].m_mapFinished[6] = true;					
				}
				if ( (i + cThis->m_pthreadnum) >= nPolocyNum ) {	
					/* 
					 当前线程被分配的最后一个任务已处理完;
					 任务启动时间检验单位为: 分钟;
					*/
                    for(int i = 0; i < (60)/5; i++)
                    {
                        sleep(5);
                        if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                            break;
                    }
				}
			}
			sleep(1); 
		}
	}
	--cThis->m_pthreadnum_alive;
 	INSPECTOR_INFO_S(OnInspectorHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CInspectorMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



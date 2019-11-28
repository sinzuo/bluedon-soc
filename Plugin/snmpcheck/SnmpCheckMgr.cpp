
#include "SnmpCheckMgr.h"
#include "../../common/BDScModuleBase.h"
#include "../../utils/bdstring.h"
#include "../share/snmp/bd_snmp_interface.h"
#include  "unistd.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CSnmpCheckMgr::CSnmpCheckMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_snmpCheckConfig,0,sizeof(m_snmpCheckConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CSnmpCheckMgr::~CSnmpCheckMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CSnmpCheckMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_snmpCheckConfig.chLog4File) > 0 ) {
        if(access(m_snmpCheckConfig.chLog4File, F_OK) != 0) { //"chLog4File" file should be existence
            return false;
	    }
		else {
	        try { 
                log4cxx::PropertyConfigurator::configure(m_snmpCheckConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    }
	else {
        return false;
	}
	
    SNMPCHECK_INFO_S( CSnmpCheckMgr::Init() successed...);
	printConfig();

    return true;
	
}
//启动模块
bool CSnmpCheckMgr::Start(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);

    /* for test.....
     *string test = "172.16.12.153~172.16.33.3~172.16.33.4~172.16.12.210";
     *SetData(NULL,&test,0);
     */
    if (! m_bIsRunning )
    {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        for (int i=0; i<m_pthreadnum; i++)
        {
            tag_snmpCheck_threadpool_t temp;
            memset(&temp, 0, sizeof(temp));
            temp.t_num = i;
            m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnSnmpCheckHandle, this) != 0) {
                SNMPCHECK_ERROR_S(Create OnSnmpCheckHandle Thread failed!);
                return false;
            }
            usleep(10000); // 10毫秒，启动线程需要一些执行时间
            m_pthreadnum_alive++;
            SNMPCHECK_INFO_S(Create OnSnmpCheckHandle Thread successed!);
        }

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            SNMPCHECK_ERROR_S(Create OnReportHandle Thread failed!);
            return false;
        }
        usleep(1000); // 1毫秒
        SNMPCHECK_INFO_S(Create OnReportHandle Thread successed!);
    }

	m_bIsLoaded = true;
    SNMPCHECK_INFO_S(CSnmpCheckMgr::Start successed!);
    return true;
}
//停止模块
bool CSnmpCheckMgr::Stop(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

    for(int i=0; i< m_pthreadnum; i++) {
        m_vecThreadPool[i].b_state = false;
    }
    //m_bIsRunning = false;

    SNMPCHECK_INFO_S(CSnmpCheckMgr::Stop successed!);
    return true;
}
//检查模块是否运行
bool CSnmpCheckMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}


UInt32 CSnmpCheckMgr::GetModuleId(void)
{
    return m_snmpCheckConfig.nModuleId;
}
UInt16 CSnmpCheckMgr::GetModuleVersion(void)
{
    return m_snmpCheckConfig.wModuleVersion;
}
string CSnmpCheckMgr::GetModuleName(void)
{
    return m_snmpCheckConfig.chModuleName;
}


//开始（下发)任务
bool CSnmpCheckMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CSnmpCheckMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

//调用方下发数据 DONE
bool CSnmpCheckMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
    m_vecPolicy.clear();
    vector<string> vec_task_unit;
    string strRsep = m_snmpCheckConfig.chRecordSep;
    string_split(strPolicy,strRsep,vec_task_unit);

    SNMPCHECK_INFO_S(<=========SNMPCHECK PlOCY=========>);
    string strFsep = m_snmpCheckConfig.chFieldSep;
    int nNum = vec_task_unit.size();  // 内容长度
    if(nNum >= 2)
    {
        vector<string> vec_numAndTime;
        //任务数量~时间间隔(s)
        string_split(vec_task_unit[0],strFsep,vec_numAndTime);
        m_time_interval = atoi(vec_numAndTime[1].c_str());
        if(m_time_interval == 0)
        {
            SNMPCHECK_ERROR_S(Invalid time_interva...);
            m_time_interval = 30;
        }

    }else
        SNMPCHECK_ERROR_S(Invalid task string...);

    for( int i=1; i<nNum; i++)
    {
        SNMPCHECK_INFO_V(" SNMPCHECK POlOCY =>" + vec_task_unit[i]);
        tag_snmpCheck_policy_t tag_policy;
        vector<string> vec_temp;
        //string strFsep = m_snmpCheckConfig.chFieldSep;
        // IP1~请求参数串(核查标识(1111)~IP~团体名~版本号~安全等级~用户名~认证方式~认证密码~加密协议~加密密码)
        string_split(vec_task_unit[i],strFsep,vec_temp);

        if( vec_temp.size() < 11)
        {
            SNMPCHECK_ERROR_S(Invalid policy string...);
            return false;
        } else
        {
            tag_policy.vec_options = vec_temp;
        }
        m_vecPolicy.push_back(tag_policy);
    }
    SNMPCHECK_INFO_S(<=========SNMPCHECK PlOCY=========>);

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_snmpCheckConfig.nTaskNumPerThread);

    //if ( m_pthreadnum == 0 ) m_pthreadnum = 1;
    m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间

    char chThreadNum[10] = {0};
    sprintf(chThreadNum, "%d", m_pthreadnum);
    SNMPCHECK_DEBUG_V("m_pthread number = " + string(chThreadNum));

    return true;
}

//调用方获取数据
void * CSnmpCheckMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//调用方释放获取到的内存
void CSnmpCheckMgr::FreeData(void * pData)
{
    return;
}

//set callback for report function
void CSnmpCheckMgr::SetReportData(pFunc_ReportData pCbReport)
{
    m_pFuncReportData = pCbReport;
}

//set call back for fetch function
void CSnmpCheckMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
    m_pFuncFetchData = pCbFetch;
}

//导入配置文件, 在Init()函数中被调用
bool CSnmpCheckMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
        cerr << "SNMPCHECK LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
        cerr << "SNMPCHECK LoadConfig:" << exc.displayText() << endl;
		return false;
	}

	try {
        GET_CONF_ITEM_CHAR(log4configfile,m_snmpCheckConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_snmpCheckConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_snmpCheckConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_snmpCheckConfig.wModuleVersion);
        GET_CONF_ITEM_CHAR(record_separator,m_snmpCheckConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_snmpCheckConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(tasknum_perthread,m_snmpCheckConfig.nTaskNumPerThread);
        if( m_snmpCheckConfig.nTaskNumPerThread <= 0 ) {
            m_snmpCheckConfig.nTaskNumPerThread = 1;  // 默认每个线程处理1个IP
		}
	}
	catch (NotFoundException& exc ) {
        cerr << "SNMPCHECK LoadConfig:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
        cerr << "SNMPCHECK LoadConfig:" << exc.displayText() << endl;
		return false;
	}
    return true;
}

//输出配置信息到日志文件, 在Init()函数中被调用
void CSnmpCheckMgr::printConfig()
{
    SNMPCHECK_INFO_S(<=========netservice configure information=========>);
    SNMPCHECK_INFO_V(" log4configfile=>    ["+string(m_snmpCheckConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",m_snmpCheckConfig.nModuleId);
    SNMPCHECK_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
    SNMPCHECK_INFO_V(" modulename=>        ["+string(m_snmpCheckConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
    sprintf(chVersion,"%d",m_snmpCheckConfig.wModuleVersion);
    SNMPCHECK_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
    SNMPCHECK_INFO_V(" record_separator=>  ["+string(m_snmpCheckConfig.chRecordSep)+string("]"));
    SNMPCHECK_INFO_V(" field_separator=>   ["+string(m_snmpCheckConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
    sprintf(chTemp,"%d",m_snmpCheckConfig.nTaskNumPerThread);
    SNMPCHECK_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    SNMPCHECK_INFO_S(<=========netservice configure information=========>);
}

bool CSnmpCheckMgr::Load(void)
{
	return true;
}

//Report Data, 用于OnReportHandle()函数
bool CSnmpCheckMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_snmpCheckConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CSnmpCheckMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}


void *CSnmpCheckMgr::OnFetchHandle(void *arg)
{

	return NULL;
}

//上报snmpCheck结果
void *CSnmpCheckMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CSnmpCheckMgr *cThis = reinterpret_cast<CSnmpCheckMgr*>(arg);

    while(true)
    {
        if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive )
        {
            cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
        }
        vector<tag_snmpCheck_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++)
        {
            modintf_datatype_t temp;
            temp.nDataType	   = MODULE_DATA_PERFORMANCE;
            temp.nMainCategory = MODULE_DATA_PERFORMANCE_SNMPCHECK;
            int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i < nChildTaskNum; i++)
            {
                if( iter_temp->m_mapFinished[i] ) // 任务完成
                {
                    if(! cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) )
                    {
                        SNMPCHECK_DEBUG_S(Report data failed!);
                    } else
                    {
                        SNMPCHECK_DEBUG_S(Report data success!);
                    }
                    iter_temp->m_mapFinished[i] = false;  // 成功或失败下次都重新执行任务
                }
            }
        }//end for(iter_temp)
        if(cThis->m_pthreadnum_alive == 0 )  //处理线程池全部退出后则退出该线程
            break;
        if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
    cThis->m_bIsRunning = false;
    cThis->m_vecThreadPool.clear();
    SNMPCHECK_INFO_S(OnReportHandle Thread exit...);

    return NULL;

}
// TODO
void* CSnmpCheckMgr::OnSnmpCheckHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CSnmpCheckMgr *cThis = reinterpret_cast<CSnmpCheckMgr*>(arg);

    int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
    cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
    int nPolocyNum = cThis->m_vecPolicy.size();

    while( cThis->m_vecThreadPool[thread_num].b_state)
    {
        if(cThis->m_bIsRunning)
        {
            for(int i = thread_num; i < nPolocyNum; i += cThis->m_pthreadnum)
            {
                //int i = m - 1;
                //if(i < 0) continue;
                vector<string> &v_options_tmp = cThis->m_vecPolicy[i].vec_options;
                int nVersion = atoi(v_options_tmp[4].c_str());
                int nSeculevel = atoi(v_options_tmp[5].c_str());
                //IP, Community, Version(int), Seculevel(int), Username, Mode, Password, privProtol, privPwd
                CSnmpV1 SnmpCheckTask(v_options_tmp[2].c_str(), v_options_tmp[3].c_str(), nVersion, nSeculevel, v_options_tmp[6].c_str(),
                                          v_options_tmp[7].c_str(), v_options_tmp[8].c_str(), v_options_tmp[9].c_str(), v_options_tmp[10].c_str());

                string strTag = v_options_tmp[1]; //标识字段符
                if(strTag.length() < 4)
                {
                    SNMPCHECK_ERROR_S(Get StringTag Length Error!);
                    continue;
                }

                if ( strTag[0] == '1' && !cThis->m_vecPolicy[i].m_mapFinished[0] )
                { // NetWorkInfoList
                    SNMPCHECK_INFO_V(string("Ready to get NetWorkInfoList from ") + v_options_tmp[0]);
                    NetWorkInfoList vecNList;
                    NetWorkInfoList vecNList_new;
                    int flag = 0;//vecNList_new下标，用于计算速率
                    if ( SnmpCheckTask.SNMP_GetNetworkList(vecNList))
                    {
                        SNMPCHECK_ERROR_S(Get Service NetWorkInfoList failed!);
                    }
                    sleep(1);
                    if ( SnmpCheckTask.SNMP_GetNetworkList(vecNList_new))//隔1秒获取两次流量信息，用于计算速率
                    {
                        SNMPCHECK_ERROR_S(Get Service NetWorkInfoList failed!);
                    }
                    // IP~1|网关名称1~网关mac地址~网关流量状态(up/down)~下行流量~上行流量~下行速率~上行速率|...
                    string strResult = v_options_tmp[0];
                    strResult += cThis->m_snmpCheckConfig.chFieldSep;
                    strResult += "1";
                    strResult += cThis->m_snmpCheckConfig.chRecordSep;

                    vector<NetworkInfo>::iterator iter = vecNList.begin();
                    for(; iter != vecNList.end(); )
                    {
                        strResult += iter->if_name;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        strResult += iter->if_mac;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        if(iter->if_status == 1) //1:up 2:down
                            strResult += "up";
                        else if(iter->if_status == 2)
                            strResult += "down";
                        else
                            SNMPCHECK_ERROR_S(Get NetworkInfo::if_status failed!);
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        char chOctet[128] = {0};
                        sprintf(chOctet, "%lld", iter->if_inOctet);
                        strResult += chOctet;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        sprintf(chOctet, "%lld", iter->if_outOctet);
                        strResult += chOctet;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;
                        //流入速率
                        sprintf(chOctet, "%lld", (vecNList_new[flag].if_inOctet-iter->if_inOctet) );
                        strResult += chOctet;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;
                        //流出速率
                        sprintf(chOctet, "%lld", (vecNList_new[flag].if_outOctet-iter->if_outOctet) );
                        strResult += chOctet;

                        if(++iter != vecNList.end())
                            strResult += cThis->m_snmpCheckConfig.chRecordSep;

                        flag++;
                    } //end for(iter)

                    cThis->m_vecPolicy[i].m_mapResult[0] = strResult;
                    cThis->m_vecPolicy[i].m_mapFinished[0] = true;
                    SNMPCHECK_DEBUG_V(strResult);
                }

                if ( strTag[1] == '1' && !cThis->m_vecPolicy[i].m_mapFinished[1] )
                { // SWInstallInfoList
                    SNMPCHECK_INFO_V(string("Ready to get SWInstallInfoList from ") + v_options_tmp[0]);
                    SWInstallInfoList vecNList;

                    if ( SnmpCheckTask.SNMP_GetSWInstalledList(vecNList))
                    {
                        SNMPCHECK_ERROR_S(Get Service SWInstallInfoList failed!);
                    }

                    // IP~2|安装服务名称~类型(unknown/operatingSystem/deviceDriver/application)~安装时间|...
                    string strResult = v_options_tmp[0];
                    strResult += cThis->m_snmpCheckConfig.chFieldSep;
                    strResult += "2";
                    strResult += cThis->m_snmpCheckConfig.chRecordSep;

                    char sw_ty[4][32]= {"unknown", "operatingSystem", "deviceDriver", "application"};

                    vector<SWInstallInfo>::iterator iter = vecNList.begin();
                    for(; iter != vecNList.end(); )
                    {
                        strResult += iter->sw_name;
                        strResult += " ";
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        if(iter->sw_type > 0 && iter->sw_type < 5)
                            strResult += sw_ty[iter->sw_type - 1];
                        else
                            SNMPCHECK_ERROR_S(Get SWInstallInfo::sw_type failed!);
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        strResult += iter->sw_date;

                        if(++iter != vecNList.end())
                            strResult += cThis->m_snmpCheckConfig.chRecordSep;
                    } //end for(iter)

                    cThis->m_vecPolicy[i].m_mapResult[1] = strResult;
                    cThis->m_vecPolicy[i].m_mapFinished[1] = true;
                    SNMPCHECK_DEBUG_V(strResult);
                }

                if ( strTag[2] == '1' && !cThis->m_vecPolicy[i].m_mapFinished[2] )
                { // ProcessInfoList
                    SNMPCHECK_INFO_V(string("Ready to get ProcessInfoList from ") + v_options_tmp[0]);
                    ProcessInfoList vecNList;
                    if ( SnmpCheckTask.SNMP_GetSWRunStatusList(vecNList))
                    {
                        SNMPCHECK_ERROR_S(Get Service ProcessInfoList failed!);
                    }

                    // IP~3|服务名称~程序路径~服务状态(running/stopped)|...
                    string strResult = v_options_tmp[0];
                    strResult += cThis->m_snmpCheckConfig.chFieldSep;
                    strResult += "3";
                    strResult += cThis->m_snmpCheckConfig.chRecordSep;

                    vector<ProcessInfo>::iterator iter = vecNList.begin();
                    for(; iter != vecNList.end(); )
                    {
                        strResult += iter->process_name;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        strResult += iter->process_path;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        if(iter->process_status == 1)
                            strResult += "running";
                        else if(iter->process_status > 1 && iter->process_status < 5)
                            strResult += "stopped";
                        else
                            SNMPCHECK_ERROR_S(Get ProcessInfo::process_status failed!);

                        if(++iter != vecNList.end())
                            strResult += cThis->m_snmpCheckConfig.chRecordSep;
                    } //end for(iter)

                    cThis->m_vecPolicy[i].m_mapResult[2] = strResult;
                    cThis->m_vecPolicy[i].m_mapFinished[2] = true;
                    SNMPCHECK_DEBUG_V(strResult);
                }

                if ( strTag[3] == '1' && !cThis->m_vecPolicy[i].m_mapFinished[3] )
                { // DeviceList
                    SNMPCHECK_INFO_V(string("Ready to get DeviceList from ") + v_options_tmp[0]);
                    DeviceList vecNList;
                    if ( SnmpCheckTask.SNMP_GetDeviceList(vecNList))
                    {
                        SNMPCHECK_ERROR_S(Get Service DeviceList failed!);
                    }

                    // IP~4|设备名称1~设备型号|设备名称2~设备型号|...
                    string strResult = v_options_tmp[0];
                    strResult += cThis->m_snmpCheckConfig.chFieldSep;
                    strResult += "4";
                    strResult += cThis->m_snmpCheckConfig.chRecordSep;

                    vector<DeviceInfo>::iterator iter = vecNList.begin();
                    for(; iter != vecNList.end(); )
                    {
                        strResult += iter->device_name;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        strResult += iter->device_type;

                        if(++iter != vecNList.end())
                            strResult += cThis->m_snmpCheckConfig.chRecordSep;
                    } //end for(iter)


                    cThis->m_vecPolicy[i].m_mapResult[3] = strResult;
                    cThis->m_vecPolicy[i].m_mapFinished[3] = true;
                    SNMPCHECK_DEBUG_V(strResult);
                }

                //此功能暂时不用
                /*
                if ( !v_options_tmp[1].compare("disk") && !cThis->m_vecPolicy[i].m_mapFinished[4] )
                { // DiskInfoList
                    SNMPCHECK_INFO_V(string("Ready to get DiskInfoList from ") + v_options_tmp[0]);
                    DiskInfoList vecNList;
                    if ( SnmpCheckTask.SNMP_GetDiskUsageList(vecNList))
                    {
                        SNMPCHECK_ERROR_S(Get Service DiskInfoList failed!);
                    }

                    // IP~5|磁盘名称~磁盘类型~磁盘使用率(整数)|...
                    string strResult = v_options_tmp[0];
                    strResult += cThis->m_snmpCheckConfig.chFieldSep;
                    strResult += "5";
                    strResult += cThis->m_snmpCheckConfig.chRecordSep;

                    vector<PartitionInfo>::iterator iter = vecNList.begin();
                    for(; iter != vecNList.end(); )
                    {
                        strResult += iter->partition_name;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        strResult += iter->partition_type;
                        strResult += cThis->m_snmpCheckConfig.chFieldSep;

                        char chUsage[8] = {0};
                        sprintf(chUsage, "%d", (int)(iter->partition_usage * 100));
                        strResult += chUsage;

                        if(++iter != vecNList.end())
                            strResult += cThis->m_snmpCheckConfig.chRecordSep;
                    } //end for(iter)


                    cThis->m_vecPolicy[i].m_mapResult[4] = strResult;
                    cThis->m_vecPolicy[i].m_mapFinished[4] = true;
                }*/

                if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                    break;

                if ( (i + cThis->m_pthreadnum) >= nPolocyNum ) {
                    /*
                     当前线程被分配的最后一个任务已处理完;
                     任务启动时间检验单位为: 秒;
                    */
                    for(int i = 0; i < (cThis->m_time_interval)/5; i++)
                    {
                        sleep(5);
                        if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                            break;
                    }
                }
            }
            sleep(10);
        } //end if(cThis->m_bIsRunning)
    } //end while(true)
    -- cThis->m_pthreadnum_alive;
    SNMPCHECK_INFO_S(OnSnmpCheckHandle Thread exit...);

    return NULL;
}

//base64解码
string CSnmpCheckMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



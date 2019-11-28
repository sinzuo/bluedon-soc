
#include "TopuMgr.h"
#include "utils/bdstring.h"
#include "utils/bd_common.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;


CTopuMgr::CTopuMgr(const string &strConfigName)
{
    m_strConfigName = strConfigName;
    memset(&m_topuConfig,0,sizeof(m_topuConfig));
    m_pFuncReportData  = NULL;
    m_pFuncFetchData   = NULL;
    m_bIsRunning       = false;
    m_pthreadnum       = 0;
    m_pthreadnum_alive = 0;

    m_vecPolicy.clear();
    m_vecThreadPool.clear();

    //设备信息线程变量
    m_host_bIsRunning  = false;
    m_host_pthreadnum       = 0;
    m_host_pthreadnum_alive = 0;
    m_host_vecPolicy.clear();
    m_host_vecThreadPool.clear();
}

CTopuMgr::~CTopuMgr(void)
{
    m_vecPolicy.clear();
    m_vecThreadPool.clear();
    //设备信息线程变量
    m_host_vecPolicy.clear();
    m_host_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CTopuMgr::Init(void)
{
    if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
    }

    if(!LoadConfig()) {
        return false;
    }

    if( strlen(m_topuConfig.chLog4File) > 0 ) {
        if(access(m_topuConfig.chLog4File, F_OK) != 0) {
            return false;
        } else {
            try {
                log4cxx::PropertyConfigurator::configure(m_topuConfig.chLog4File);
                //g_logger = Logger::getLogger("LOG4_PROXY");
            } catch (Exception &) {
                return false;
            }
        }
    } else {
        return false;
    }

    TOPU_INFO_S( CTopuMgr::Init() successed...);
    printConfig();
    /*test*/
    //Start();
    return true;
}

bool CTopuMgr::IsRunning(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
    return m_bIsRunning;
}
UInt32 CTopuMgr::GetModuleId(void)
{
    return m_topuConfig.nModuleId;
}
UInt16 CTopuMgr::GetModuleVersion(void)
{
    return m_topuConfig.wModuleVersion;
}
string CTopuMgr::GetModuleName(void)
{
    return m_topuConfig.chModuleName;
}

//开始（下发)任务
bool CTopuMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;
}

//停止（取消）任务
bool CTopuMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;
}

bool CTopuMgr::LoadConfig(void)
{
    AutoPtr<IniFileConfiguration> pConf = NULL;
    try {
        pConf = new IniFileConfiguration(m_strConfigName);
    } catch (NotFoundException& exc ) {
        cerr << "TOPU LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;
    } catch (SyntaxException& exc) {
        cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;
    } catch (Exception& exc) {
        cerr << "TOPU LoadConfig:" << exc.displayText() << endl;
        return false;
    }

    try {
        GET_CONF_ITEM_CHAR(log4configfile,m_topuConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_topuConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_topuConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_topuConfig.wModuleVersion);
        GET_CONF_ITEM_CHAR(record_separator,m_topuConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_topuConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(tasknum_perthread,m_topuConfig.nTaskNumPerThread);
        if( m_topuConfig.nTaskNumPerThread <= 0 ) {
            m_topuConfig.nTaskNumPerThread = 60;  // 默认每个线程处理60个IP
        }
    }
    catch (NotFoundException& exc ) {
        cerr << "TOPU LoadConfig:" << exc.displayText() << endl;
        return false;
    } catch (SyntaxException& exc) {
        cerr << "TOPU LoadConfig:" << exc.displayText() << endl;
        return false;
    }
    return true;
}

void CTopuMgr::printConfig() {
    TOPU_INFO_S(<=========netservice configure information=========>);
    TOPU_INFO_V(" log4configfile=>    ["+string(m_topuConfig.chLog4File)+string("]"));
    char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",m_topuConfig.nModuleId);
    TOPU_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
    TOPU_INFO_V(" modulename=>        ["+string(m_topuConfig.chModuleName)+string("]"));
    char chVersion[10] = {0};
    sprintf(chVersion,"%d",m_topuConfig.wModuleVersion);
    TOPU_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
    TOPU_INFO_V(" record_separator=>  ["+string(m_topuConfig.chRecordSep)+string("]"));
    TOPU_INFO_V(" field_separator=>   ["+string(m_topuConfig.chFieldSep)+string("]"));
    char chTemp[5] = {0};
    sprintf(chTemp,"%d",m_topuConfig.nTaskNumPerThread);
    TOPU_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    TOPU_INFO_S(<=========netservice configure information=========>);
}

bool CTopuMgr::Load(void)
{
    return true;
}

//调用方下发数据     交换机IP~团体名~版本号~安全等级~用户名~认证方式~认证密码~加密协议~加密密码~采集频率
//v3               172.16.7.1~public~snmpv3~MD5~bluedonpw~3600~v3
bool CTopuMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
    m_vecPolicy.clear();
    m_vecThreadPool.clear();
    vector<string> vec_temp;
    //string strRsep = m_topuConfig.chRecordSep;
     string strFsep = m_topuConfig.chFieldSep;
    string_split(strPolicy,strFsep,vec_temp);

    TOPU_INFO_S(<=======TOPU PlOCY=======>);

    TOPU_INFO_V(" TOPU POlOCY =>" + strPolicy);
    tag_topu_policy_t tag_policy;
    if(vec_temp.size() != 10) {
        TOPU_ERROR_S(Invalid polocy string...);
        return false;
    }
    else{
        tag_policy.vec_options = vec_temp;
        this->m_conn_info.pszHost = vec_temp[0];
        this->m_conn_info.pszCommunity = vec_temp[1];
        this->m_conn_info.lVersion = atol(vec_temp[2].c_str());
        this->m_conn_info.secuLevel = atoi(vec_temp[3].c_str());
        this->m_conn_info.pszUser = vec_temp[4];
        this->m_conn_info.pszProtol = vec_temp[5];
        this->m_conn_info.pszPassPhrase = vec_temp[6];
        this->m_conn_info.privProtol = vec_temp[7];
        this->m_conn_info.privPassPhrase = vec_temp[8];
        this->m_conn_info.freq = atoi(vec_temp[9].c_str());

       }
    m_vecPolicy.push_back(tag_policy);

    TOPU_INFO_S(<=======TOPU PlOCY=======>);

    //m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_topuConfig.nTaskNumPerThread);
    m_pthreadnum = 1; //一个线程获取拓扑信息，再分线程获取主机信息
    m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间

    return true;
}
//调用方获取数据
void * CTopuMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//调用方释放获取到的内存
void CTopuMgr::FreeData(void * pData)
{
    return;
}

//set callback for report function
void CTopuMgr::SetReportData(pFunc_ReportData pCbReport)
{
    m_pFuncReportData = pCbReport;
}
//set call back for fetch function
void CTopuMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
    m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CTopuMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_topuConfig.nModuleId,pDataType,pData,dwLength);
    }
     return true;
}
//Fetch Data
const char* CTopuMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
    return NULL;
}

//启动模块
bool CTopuMgr::Start(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);
/*test*/
    //string str = "172.16.12.254~public~root~md5~dev188~1";
    //string str = "172.16.12.254~public~161~3600";
    //SetData(NULL,&str,str.length());
/**/
    if (! m_bIsRunning )
    {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        tag_topu_threadpool_t temp;
        memset(&temp, 0, sizeof(temp));
        temp.t_num = 0; //获取拓扑关系只有一个线程
        m_vecThreadPool.push_back(temp);

        if (pthread_create(&(m_vecThreadPool[0].t_id), &pa, OnTopuHandle, this) != 0)
        {
            TOPU_ERROR_S(Create OnTopuHandle Thread failed!);
            return false;
        }
        usleep(10000); // 10毫秒，启动线程需要一些执行时间
        m_pthreadnum_alive++;

        TOPU_INFO_S(Create OnTopuHandle Thread successed!);

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0)
        {
            TOPU_ERROR_S(Create OnReportHandle Thread failed!);
            return false;
        }
        usleep(10000); // 10毫秒
        TOPU_INFO_S(Create OnReportHandle Thread successed!);
    }
    TOPU_INFO_S(CTopuMgr::Start successed!);
    return true;
}

//停止模块
bool CTopuMgr::Stop(void)
{
    FastMutex::ScopedLock lock(m_ModMutex);

    m_vecThreadPool[0].b_state = false;

    for(int i=0; i< m_host_pthreadnum; i++) {
        m_host_vecThreadPool[i].b_state = false;
    }
    TOPU_INFO_S(CTopuMgr::Stop successed!);
    return true;
}

void *CTopuMgr::OnFetchHandle(void *arg) {
    return NULL;
}

// 上报任务处理结果
void *CTopuMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CTopuMgr *cThis = reinterpret_cast<CTopuMgr*>(arg);
    int Freq = cThis->m_conn_info.freq;  //采集频率
    while (true)
    {
        if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive )
        {
            cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
        }

        if ( cThis->m_host_pthreadnum == cThis->m_host_pthreadnum_alive )
        {
            if(cThis->m_host_pthreadnum != 0)
            {
                //TOPU_DEBUG_S(Report data success!);
                cThis->m_host_bIsRunning = true; //所有设备获取线程已经建立完毕
            }

        }

        if(cThis->m_host_bIsRunning == true)
        {
            bool all_finished = true;  //控制是否全部获取主机信息已经完成
            vector<tag_topu_policy_t>::iterator iter_host_temp = cThis->m_host_vecPolicy.begin();

            for( int i = 0;iter_host_temp != cThis->m_host_vecPolicy.end(); iter_host_temp++)
            {

                if( !iter_host_temp->m_mapFinished[0] )// 任务完成
                {
                    TOPU_INFO_V(string("m_mapFinished false =  ") + bd_int2str(i));
                    all_finished = false;
                    break;
                }
                i++;
            }

            if(all_finished)
            {
                iter_host_temp = cThis->m_host_vecPolicy.begin();//指向list的开头
                int nhostNum = cThis->m_host_vecPolicy.size();
                (iter_host_temp+nhostNum -1)->m_mapResult[0] += "|end"; // 最后加上"|end"
                for( ;iter_host_temp != cThis->m_host_vecPolicy.end(); iter_host_temp++)
                {
                    modintf_datatype_t temp;
                    temp.nDataType	   = MODULE_DATA_SERVICE;
                    temp.nMainCategory = MODULE_DATA_SERVICE_NETTOPO;
                    int nChildTaskNum = iter_host_temp->m_mapFinished.size();
                    for(int i=0 ; i < nChildTaskNum; i++) {
                        if( iter_host_temp->m_mapFinished[i] )
                        { // 任务完成,发送主机信息
                            TOPU_DEBUG_V( string("send dev data = ")+ iter_host_temp->m_mapResult[i]);//test
                            if(! cThis->ReportData(&temp,(void *)&(iter_host_temp->m_mapResult[i]),iter_host_temp->m_mapResult[i].length()) ) //
                            {
                                TOPU_DEBUG_S(Report data failed!);
                            }
                            else
                            {
                                TOPU_DEBUG_S(Report data success!);
                            }
                            iter_host_temp->m_mapFinished[i] = false;  // 成功或失败下次都重新执行任务
                        }
                    }
                }

                int j = 0;
                cThis->m_topu_vecPolicy.m_mapFinished.clear();
                cThis->m_topu_vecPolicy.m_mapResult.clear();

                for (vector<L2_TOPO_INFO>::iterator it_c = cThis->l2_topuinfo_list.begin(); it_c != cThis->l2_topuinfo_list.end(); ++it_c)
                {

                    int nPolocyNum = cThis->hostinfo_vec.size();
                    int sub_id = 0;
                    int parent_id = 0;
                    string sub_name;
                    string parent_name;
                    int sub_type = 0;
                    int parent_type = 0;
                    for(int i=0 ; i< nPolocyNum; i++)
                    {
                        if(cThis->hostinfo_vec[i].sysip == it_c->Sub_Node_IP)
                        {
                            sub_id = cThis->hostinfo_vec[i].recordid;
                            sub_name = cThis->hostinfo_vec[i].sysname;
                            sub_type = cThis->hostinfo_vec[i].NEW_Dev_Type;
                            break;
                        }

                    }
                    for(int i=0 ; i< nPolocyNum; i++)
                    {
                        if(cThis->hostinfo_vec[i].sysip == it_c->Parent_Node_IP)
                        {
                            parent_id = cThis->hostinfo_vec[i].recordid;
                            parent_name = cThis->hostinfo_vec[i].sysname;
                            parent_type = cThis->hostinfo_vec[i].NEW_Dev_Type;
                            break;
                        }
                    }
                    /**/
                    string strResult = "nettopu";
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_ID";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += bd_int2str(sub_id);
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_Name";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += sub_name;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_IP";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += it_c->Sub_Node_IP;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_IF_Name";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += it_c->Sub_Node_IF_Name;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_IF_MAC";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += it_c->Sub_Node_IF_MAC;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_Name";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += parent_name;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_IP";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += it_c->Parent_Node_IP;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_IF_Name";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += it_c->Parent_Node_IF_Name;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_IF_MAC";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += it_c->Parent_Node_IF_MAC;
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_IF_Index";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += bd_int2str(it_c->Sub_Node_IF_Index);
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Sub_Node_Dev_Type";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += bd_int2str(sub_type);
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_ID";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += bd_int2str(parent_id);
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_IF_Index";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += bd_int2str(it_c->Parent_Node_IF_Index);
                    strResult += cThis->m_topuConfig.chRecordSep;
                    strResult += "Parent_Node_Dev_Type";
                    strResult += cThis->m_topuConfig.chFieldSep;
                    strResult += bd_int2str(parent_type);
                    //TOPU_INFO_V(strResult);//test
                    cThis->m_topu_vecPolicy.m_mapResult[j] = strResult;
                    cThis->m_topu_vecPolicy.m_mapFinished[j] = true;
                    j++;
                }



                modintf_datatype_t temp;
                temp.nDataType	   = MODULE_DATA_SERVICE;
                temp.nMainCategory = MODULE_DATA_SERVICE_NETTOPO;

                if( cThis->m_vecPolicy[0].m_mapFinished[0] ) { // 任务完成
                    // 拓扑任务
                    int nChildTaskNum = cThis->m_topu_vecPolicy.m_mapFinished.size();
                    cThis->m_topu_vecPolicy.m_mapResult[nChildTaskNum -1] += "|end~yes"; // 最后加上"|end~yes"
                    for(int j=0 ; j < nChildTaskNum; j++) {
                        if( cThis->m_topu_vecPolicy.m_mapFinished[j] )
                        { // 任务完成
                           TOPU_DEBUG_V( string("send topu data = ")+ cThis->m_topu_vecPolicy.m_mapResult[j]);//test
                            if(! cThis->ReportData(&temp,(void *)&(cThis->m_topu_vecPolicy.m_mapResult[j]),cThis->m_topu_vecPolicy.m_mapResult[j].length()) )
                            {
                                TOPU_DEBUG_S(Report data failed!);
                            }
                            else
                            {
                                TOPU_DEBUG_S(Report data success!);
                            }
                            cThis->m_topu_vecPolicy.m_mapFinished[j] = false;  // 成功或失败下次都重新执行任务
                        }
                    }

                }

              //  sleep(20);
                int Freqtime = Freq/5;
                for(int i = 0; i < Freqtime ;i++)
                {
                    sleep(5);
                   // TOPU_INFO_S(sleep);
                    if(!cThis->m_vecThreadPool[0].b_state) break;
                }

                cThis->m_vecPolicy[0].m_mapFinished[0] = false;  // 成功或失败下次都重新执行任务
               // cThis->m_bIsRunning = false;// 发送信息后重置状态
               // cThis->m_host_bIsRunning = false;//发送信息后重置状态


            }

        }
        sleep(10);

        if(cThis->m_pthreadnum_alive == 0  && cThis->m_host_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程
        if(cThis->m_host_bIsRunning || cThis->m_bIsRunning ) sleep(1); // 不用一直扫描

    }

    cThis->m_host_bIsRunning = false;
    cThis->m_bIsRunning = false;
	cThis->m_vecThreadPool.clear();
    cThis->m_host_vecPolicy.clear();
    cThis->m_host_vecThreadPool.clear();
    TOPU_INFO_S(OnReportHandle Thread exit...);

    return NULL;

}

void* CTopuMgr::OnTopuHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CTopuMgr *cThis = reinterpret_cast<CTopuMgr*>(arg);

    cThis->m_vecThreadPool[0].b_state = true;   // 获取拓扑关系只有一个线程,线程状态修改为启动

    while( cThis->m_vecThreadPool[0].b_state)
    {
        if(cThis->m_bIsRunning)
        {
            //vector<string> v_options_tmp = cThis->m_vecPolicy[0].vec_options;
            string cur_ip = cThis->m_conn_info.pszHost; //当前路由ip
            list<L3_TOPO_INFO> l3_topuinfo_list;//保存拓扑信息
            map<string, string> all_topu_ip;

            CTopu TopuTask(cThis->m_conn_info.pszHost, cThis->m_conn_info.pszCommunity, \
			               cThis->m_conn_info.lVersion, cThis->m_conn_info.secuLevel , \
						   cThis->m_conn_info.pszUser,cThis->m_conn_info.pszProtol, \
						   cThis->m_conn_info.pszPassPhrase, cThis->m_conn_info.privProtol, cThis->m_conn_info.privPassPhrase);//利用参数初始化snmp

            set<string> L3routelist;//记录能访问的路由，用于获取子网设备

            if ( !cThis->m_vecPolicy[0].m_mapFinished[0] )//主拓扑
            {
                TOPU_INFO_V(string("Ready to get host info from ") + cur_ip);
                HOSTINFO devhostinfo;
                //devhostinfo.sysip = cur_ip;//ip
                if ( !TopuTask.get_info_from_L3(devhostinfo))
                {
                    TOPU_ERROR_S(Get host info failed!);
                }

                if (devhostinfo.SNMP_ENABLE != 1 || devhostinfo.ifrouter != 1) //不是路由设备
                {

                    TOPU_ERROR_S("This host not a route!");
                    cThis->m_vecThreadPool[0].b_state = false;
                    break;

                } else  //主业务入口
                {
                    if ( !TopuTask.get_L3_info_from_ipRouteTable(l3_topuinfo_list, L3routelist))
                    {
                        TOPU_ERROR_S(Get ipRouteTable info failed!);
                    }
                    cThis->l2_topuinfo_list.clear();//清空

                    if (!TopuTask.get_L2info_from_ipNetToMediaTable(L3routelist, l3_topuinfo_list, cThis->l2_topuinfo_list, all_topu_ip))
                    {
                        TOPU_ERROR_S(Get ipNetToMediaTable info failed!);
                    }


                    cThis->m_host_vecPolicy.clear();
                    cThis->m_host_vecThreadPool.clear();
                    cThis->hostinfo_vec.clear();
                    cThis->m_host_vecPolicy.reserve(all_topu_ip.size());  //用于保存主机信息
                    cThis->hostinfo_vec.reserve(all_topu_ip.size());      //用于保存主机信息

                    for (map<string,string>::iterator it_c = all_topu_ip.begin(); it_c != all_topu_ip.end(); ++it_c)
                    {
                        //获取主机信息线程需要
                        tag_topu_policy_t tag_policy;
                        vector<string> vec_temp;
                        vec_temp.push_back(it_c->first);
                        vec_temp.push_back(it_c->second);
                        tag_policy.vec_options = vec_temp;
                        cThis->m_host_vecPolicy.push_back(tag_policy);

                        HOSTINFO tem_host_info;
                        tem_host_info.sysip = it_c->first;
                        cThis->hostinfo_vec.push_back(tem_host_info);
                    }

                    cThis->m_host_pthreadnum = cThis->m_host_vecPolicy.size()/cThis->m_topuConfig.nTaskNumPerThread;   //配置文件中读取线程处理的ip数量
                    if (cThis->m_host_pthreadnum == 0)
                        cThis->m_host_pthreadnum = 1;

                    cThis->m_host_vecThreadPool.reserve(cThis->m_host_pthreadnum);
                    TOPU_INFO_V(string("Ready to get host info policynum = ") + bd_int2str(cThis->m_host_vecPolicy.size()));
                    TOPU_INFO_V(string("Ready to get host info pthreadnum = ") + bd_int2str(cThis->m_host_pthreadnum));

                    for (int i_p = 0; i_p < cThis->m_host_pthreadnum; i_p++)
                    {
                        pthread_attr_t pa;       //线程属性
                        pthread_attr_init(&pa);	//初始化线程属性
                        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性

                        tag_topu_threadpool_t temp;
                        memset(&temp, 0, sizeof(temp));
                        temp.t_num = i_p;
                        cThis->m_host_vecThreadPool.push_back(temp);
                        if (pthread_create(&(cThis->m_host_vecThreadPool[i_p].t_id), &pa, OngethostHandle, cThis) != 0) {
                            TOPU_ERROR_S(Create OngethostHandle Thread failed!);
                            return NULL;
                        }
                        usleep(100000); // 10毫秒，启动线程需要一些执行时间
                        cThis->m_host_pthreadnum_alive++;
                        TOPU_INFO_S(Create OnhostHandle Thread successed!);
                    }

                    cThis->m_vecPolicy[0].m_mapFinished[0] = true;
                }

            }

        }
        sleep(10);
    }
    --cThis->m_pthreadnum_alive;
    TOPU_INFO_S(OnTopuHandle Thread exit...);

    return NULL;
}


void* CTopuMgr::OngethostHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CTopuMgr *cThis = reinterpret_cast<CTopuMgr*>(arg);

    int thread_num = cThis->m_host_vecThreadPool.size() - 1;  // 当前线程num号
    TOPU_INFO_V( string("get host info is running threadnum = ")+ bd_int2str(thread_num));//test

    cThis->m_host_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
    int nPolocyNum = cThis->m_host_vecPolicy.size();

    vector<string> v_options_tmp = cThis->m_vecPolicy[0].vec_options;//策略中的参数初始化

    //CTopu TopuTask(cThis->m_conn_info.pszHost, cThis->m_conn_info.pszCommunity, cThis->m_conn_info.pszUser,cThis->m_conn_info.pszProtol,cThis->m_conn_info.pszPassPhrase,cThis->m_conn_info.lVersion);//利用参数初始化snmp
    CTopu TopuTask(cThis->m_conn_info.pszHost, cThis->m_conn_info.pszCommunity, \
	               cThis->m_conn_info.lVersion, cThis->m_conn_info.secuLevel , \
	               cThis->m_conn_info.pszUser,cThis->m_conn_info.pszProtol, \
	               cThis->m_conn_info.pszPassPhrase, cThis->m_conn_info.privProtol, \
	               cThis->m_conn_info.privPassPhrase);
    if( cThis->m_host_vecThreadPool[thread_num].b_state)
    {
        for(int i=thread_num; i< nPolocyNum; i += cThis->m_host_pthreadnum)
        {
            if ( !cThis->m_host_vecPolicy[i].m_mapFinished[0] ) {

                TOPU_INFO_V( string("get host info is running i = ")+ bd_int2str(i)+string(" thread_num = ")+bd_int2str(thread_num));//test
                HOSTINFO hostinfo;

                hostinfo.recordid = 10000 + i; //设备id
                hostinfo.sysip = cThis->m_host_vecPolicy[i].vec_options[0];
                hostinfo.IP_Count = 1;
                hostinfo.Dev_IP.insert(cThis->m_host_vecPolicy[i].vec_options[0]);
                hostinfo.MAC_Count = 0;
				hostinfo.NEW_Dev_Type = 40000;
				
                if(cThis->m_host_vecPolicy[i].vec_options[1] == "subnet")//如果设备是子网，这设置为子网类型
                {
                    hostinfo.NEW_Dev_Type = 13000;
                }
                else if(!cThis->m_host_vecPolicy[i].vec_options[1].empty()||cThis->m_host_vecPolicy[i].vec_options[1] != "")
                {
                    hostinfo.Dev_MAC.insert(cThis->m_host_vecPolicy[i].vec_options[1]);
                    hostinfo.MAC_Count = 1;
                }



                /*
               if(!TopuTask.get_info_from_host(hostinfo, "public", "", "", "", NETSNMP_VERSION_2c))
                {
                     TOPU_INFO_V(string("fail to get host info ip =") +  hostinfo.sysip);
               }
               */

               if(!TopuTask.get_info_from_host(hostinfo,cThis->m_conn_info.pszCommunity.c_str(), \
                                               cThis->m_conn_info.lVersion, cThis->m_conn_info.secuLevel, \
                                               cThis->m_conn_info.pszUser.c_str(), cThis->m_conn_info.pszProtol.c_str(), \
                                               cThis->m_conn_info.pszPassPhrase.c_str(), cThis->m_conn_info.privProtol.c_str(), \
                                               cThis->m_conn_info.privPassPhrase.c_str()))
               {
                     TOPU_INFO_V(string("fail to get host info ip =") +  hostinfo.sysip);
               }



               /**/
               cThis->hostinfo_vec[i].recordid = hostinfo.recordid;
               cThis->hostinfo_vec[i].sysip = hostinfo.sysip;
               cThis->hostinfo_vec[i].devmac = hostinfo.devmac;
               cThis->hostinfo_vec[i].sysdescvalue = hostinfo.sysdescvalue;
               cThis->hostinfo_vec[i].Model_Number = hostinfo.Model_Number;
               cThis->hostinfo_vec[i].sysop = hostinfo.sysop;
               cThis->hostinfo_vec[i].sysuptimevalue = hostinfo.sysuptimevalue;
               cThis->hostinfo_vec[i].syscontactvalue = hostinfo.syscontactvalue;
               cThis->hostinfo_vec[i].sysname = hostinfo.sysname;
               cThis->hostinfo_vec[i].syslocationvalue = hostinfo.syslocationvalue;
               cThis->hostinfo_vec[i].Dev_Type = hostinfo.Dev_Type;
               cThis->hostinfo_vec[i].SNMP_ENABLE = hostinfo.SNMP_ENABLE;
               cThis->hostinfo_vec[i].ICMP_ENABLE = hostinfo.ICMP_ENABLE;
               cThis->hostinfo_vec[i].ifrouter = hostinfo.ifrouter;
               cThis->hostinfo_vec[i].IP_Count = hostinfo.IP_Count;
               cThis->hostinfo_vec[i].Dev_IP = hostinfo.Dev_IP;
               cThis->hostinfo_vec[i].MAC_Count = hostinfo.MAC_Count;
               cThis->hostinfo_vec[i].Dev_MAC = hostinfo.Dev_MAC;
               cThis->hostinfo_vec[i].NEW_Dev_Type = hostinfo.NEW_Dev_Type;

               /**/

               //cThis->hostinfo_vec.push_back(hostinfo);

               string strResult = "Dev";
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "ID";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += bd_int2str(hostinfo.recordid);
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "Dev_Name";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += hostinfo.sysname;
//                for (set <string>::iterator it = hostinfo.Dev_IP.begin(); it != hostinfo.Dev_IP.end(); ++it)
//                {
//                    strResult += cThis->m_topuConfig.chRecordSep;
//                    strResult += "Dev_IP";
//                    strResult += cThis->m_topuConfig.chFieldSep;
//                    strResult += (*it);
//                }
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "Dev_IP";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += hostinfo.sysip;


               for (set <string>::iterator it = hostinfo.Dev_MAC.begin(); it != hostinfo.Dev_MAC.end(); ++it)
               {
                   strResult += cThis->m_topuConfig.chRecordSep;
                   strResult += "Dev_MAC";
                   strResult += cThis->m_topuConfig.chFieldSep;
                   strResult += (*it);
               }
               if(hostinfo.Dev_MAC.size() == 0) //没有mac地址则输出一个空的Dev_MAC
               {
                   strResult += cThis->m_topuConfig.chRecordSep;
                   strResult += "Dev_MAC";
                   strResult += cThis->m_topuConfig.chFieldSep;
               }
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "Manufacturer";
               strResult += cThis->m_topuConfig.chFieldSep;
               // strResult += hostinfo.syscontactvalue;
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "Model_Number";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += hostinfo.Model_Number;
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "IP_Count";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += bd_int2str(hostinfo.IP_Count);
               //strResult += "1";
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "MAC_Count";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += bd_int2str(hostinfo.MAC_Count);
               //strResult += "0";
               strResult += cThis->m_topuConfig.chRecordSep;

               strResult += "Dev_Type";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += bd_int2str(hostinfo.NEW_Dev_Type);
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "SNMP_ENABLE";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += bd_int2str(hostinfo.SNMP_ENABLE);
               strResult += cThis->m_topuConfig.chRecordSep;
               strResult += "ICMP_ENABLE";
               strResult += cThis->m_topuConfig.chFieldSep;
               strResult += bd_int2str(hostinfo.ICMP_ENABLE);



               //TOPU_INFO_V(strResult);//test
               cThis->m_host_vecPolicy[i].m_mapResult[0] = strResult;
               cThis->m_host_vecPolicy[i].m_mapFinished[0] = true;
            }

        }
            TOPU_INFO_V( string("get host info is over = ")+ bd_int2str(thread_num));//test
       // sleep(10);
    }
    --cThis->m_host_pthreadnum_alive;
    TOPU_INFO_S(OnTopuHandle Thread exit...);

    return NULL;
}
//base64解码
string CTopuMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);

  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));

  return out.str();
}



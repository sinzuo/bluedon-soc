
#include "NmapscanMgr.h"
#include "utils/bdstring.h"
#include "utils/bd_common.h"
#include  "unistd.h"
#include <fcntl.h> 
#include <iostream>
#include <cstdio>
#include <regex.h>

using namespace Poco;
using namespace std;


CNmapscanMgr::CNmapscanMgr(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_nmapscanConfig,0,sizeof(m_nmapscanConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CNmapscanMgr::~CNmapscanMgr(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CNmapscanMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_nmapscanConfig.chLog4File) > 0 ) {
        if(access(m_nmapscanConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
                log4cxx::PropertyConfigurator::configure(m_nmapscanConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    NMAPSCAN_INFO_S( CNmapscanMgr::Init() successed...);
	printConfig();
    /*test*/
    //Start();
    return true;	
}

bool CNmapscanMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CNmapscanMgr::GetModuleId(void)
{
    return m_nmapscanConfig.nModuleId;
}
UInt16 CNmapscanMgr::GetModuleVersion(void)
{
    return m_nmapscanConfig.wModuleVersion;
}
string CNmapscanMgr::GetModuleName(void)
{
    return m_nmapscanConfig.chModuleName;
}

//开始（下发)任务
bool CNmapscanMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CNmapscanMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CNmapscanMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
        cerr << "NMAPSCAN LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
        cerr << "NMAPSCAN LoadConfig:" << exc.displayText() << endl;
		return false;
	}

	try {
        GET_CONF_ITEM_CHAR(log4configfile,m_nmapscanConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_nmapscanConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_nmapscanConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_nmapscanConfig.wModuleVersion);
        GET_CONF_ITEM_CHAR(record_separator,m_nmapscanConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_nmapscanConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(tasknum_perthread,m_nmapscanConfig.nTaskNumPerThread);
        GET_CONF_ITEM_INT(nSleepsec,m_nmapscanConfig.nSleepsec);
        if( m_nmapscanConfig.nTaskNumPerThread <= 0 ) {
            m_nmapscanConfig.nTaskNumPerThread = 100;  // 默认每个线程处理5个IP
		}
        if( m_nmapscanConfig.nSleepsec <= 0 ) {
            m_nmapscanConfig.nSleepsec = 60;  // 默认睡眠60秒
        }
	}
	catch (NotFoundException& exc ) {
        cerr << "NMAPSCAN LoadConfig:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
        cerr << "NMAPSCAN LoadConfig:" << exc.displayText() << endl;
		return false;
	}
    return true;
}

void CNmapscanMgr::printConfig() {
    NMAPSCAN_INFO_S(<=========netservice configure information=========>);
    NMAPSCAN_INFO_V(" log4configfile=>    ["+string(m_nmapscanConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",m_nmapscanConfig.nModuleId);
    NMAPSCAN_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
    NMAPSCAN_INFO_V(" modulename=>        ["+string(m_nmapscanConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
    sprintf(chVersion,"%d",m_nmapscanConfig.wModuleVersion);
    NMAPSCAN_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
    NMAPSCAN_INFO_V(" record_separator=>  ["+string(m_nmapscanConfig.chRecordSep)+string("]"));
    NMAPSCAN_INFO_V(" field_separator=>   ["+string(m_nmapscanConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
    sprintf(chTemp,"%d",m_nmapscanConfig.nTaskNumPerThread);
    NMAPSCAN_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
    NMAPSCAN_INFO_S(<=========netservice configure information=========>);
}

bool CNmapscanMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CNmapscanMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{

    /**/
    string strPolicy = *(string *)pData;
    m_vecPolicy.clear();
    m_vecThreadPool.clear();
    vector<string> vecIP;
    string strFsep = m_nmapscanConfig.chFieldSep;
    string_split(strPolicy,strFsep,vecIP);
    m_vecPolicy.reserve(vecIP.size());

    vector<string>::iterator iter = vecIP.begin();
    NMAPSCAN_INFO_S(<=========NMAPSCAN PlOCY=========>);
    for(;iter!= vecIP.end();iter++) {
        tag_nmapscan_policy_t temp;
        temp.strIP   = *iter;
        temp.nResult.empty();
        temp.nfinished = false;
        m_vecPolicy.push_back(temp);
        NMAPSCAN_INFO_V(" NMAPSCAN PlOCY =>" + temp.strIP);
    }
    NMAPSCAN_INFO_S(<=========NMAPSCAN PlOCY=========>);

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_nmapscanConfig.nTaskNumPerThread);
    m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
    /**/
	return true;
}
//调用方获取数据
void * CNmapscanMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CNmapscanMgr::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CNmapscanMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CNmapscanMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CNmapscanMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_nmapscanConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CNmapscanMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CNmapscanMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	
    //string str = "172.16.12.254~172.16.12.54~172.16.12.144";
    //SetData(NULL,&str,str.length());
 	
    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
            tag_nmapscan_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnNmapscanHandle, this) != 0) {
                NMAPSCAN_ERROR_S(Create OnNmapscanHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
            NMAPSCAN_INFO_S(Create OnNmapscanHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            NMAPSCAN_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
        NMAPSCAN_INFO_S(Create OnReportHandle Thread successed!);
	}	

	m_bIsLoaded = true;
    NMAPSCAN_INFO_S(CNmapscanMgr::Start successed!);
	return true;
}

//停止模块
bool CNmapscanMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}	
    NMAPSCAN_INFO_S(CNmapscanMgr::Stop successed!);
    return true;
}

void *CNmapscanMgr::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CNmapscanMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CNmapscanMgr *cThis = reinterpret_cast<CNmapscanMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_nmapscan_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
            temp.nMainCategory = MODULE_DATA_PERFORMANCE_NMAPSCAN;
                                //需要修改成MODULE_DATA_PERFORMANCE_NMAPSCAN

            if( iter_temp->nfinished ) { // 任务完成
            NMAPSCAN_DEBUG_V(string("send => ") + iter_temp->nResult);
            if(! cThis->ReportData(&temp, (void *)&(iter_temp->nResult), (iter_temp->nResult).length()) ) {
                NMAPSCAN_DEBUG_S(Report data failed!);
            } else {
                NMAPSCAN_DEBUG_S(Report data success!);
            }

                iter_temp->nfinished = false;  // 成功或失败下次都重新执行任务
            }
        }		
		if(cThis->m_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程	
		if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
	cThis->m_bIsRunning = false;
	cThis->m_vecThreadPool.clear();
    NMAPSCAN_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;

}

void* CNmapscanMgr::OnNmapscanHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CNmapscanMgr *cThis = reinterpret_cast<CNmapscanMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
    sleep(1);
	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {		
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
                if ( !cThis->m_vecPolicy[i].nfinished ) { // 扫描设备端口
                    string strIP = cThis->m_vecPolicy[i].strIP;
                    if(strIP.length() < 4 || strIP.empty())  //
                        continue;
                    NMAPSCAN_INFO_V(string("Ready to nmap scan  from ") + strIP);

                    FILE *fp;
                    char fstr[1024];
                    //string o_portsplit;//临时变量
                    //vector<string> vecname;
                    vector<string> vecport;
                    vector<string> port_prot_tmp;
                    vector<string> result_temp;

                    // 通过nmap扫描tcp设备端口
                    string cmd = string("nmap -T3 -sS ") + strIP;
                    if ((fp = popen(cmd.c_str(), "r")) == NULL)
                    {
                        NMAPSCAN_INFO_S(string("popen tcp error !"));
                        return NULL;
                    }
                    while (fgets(fstr, sizeof(fstr), fp) != NULL)  //寻找匹配的开放端口
                    {
                        string str;
                        regex_t regex;
                        string pattern;
                        regmatch_t match_t;
                        size_t nmatch;

                        str = string(fstr);
                        nmatch = 1;

                        /*
                        pattern = string("PORT[ ]{1,}STATE[ ]{1,}SERVICE.*");
                        if (REG_NOERROR == regcomp(&regex, pattern.c_str(), REG_EXTENDED | REG_NEWLINE))
                        {
                            if (REG_NOERROR == regexec(&regex, str.c_str(), nmatch, &match_t, 0))
                            {
                                o_portsplit = str.substr(match_t.rm_so, match_t.rm_eo - match_t.rm_so);
                                vecname.clear();
                                string strFsep = " ";
                                string_split(o_portsplit,strFsep,vecname);

                            }
                            regfree(&regex);
                        }
                        pattern = string("PORT[ ]{1,}STATE[ ]{1,}SERVICE[ ]{1,}VERSION.*");
                        if (REG_NOERROR == regcomp(&regex, pattern.c_str(), REG_EXTENDED | REG_NEWLINE))
                        {
                            if (REG_NOERROR == regexec(&regex, str.c_str(), nmatch, &match_t, 0))
                            {
                                o_portsplit = str.substr(match_t.rm_so, match_t.rm_eo - match_t.rm_so);
                                vecname.clear();
                                string strFsep = " ";
                                string_split(o_portsplit,strFsep,vecname);
                            }
                            regfree(&regex);
                        }

                        if(vecname.empty())  //nmap的抬头不能为空
                            continue;
                        */
                        pattern = string("[0-9]{1,}/(tcp|udp)[ ]{1,}(open|filtered|closed|unfiltered|\\|){1,}.*");
                        if (REG_NOERROR == regcomp(&regex, pattern.c_str(), REG_EXTENDED | REG_NEWLINE))
                        {
                            if (REG_NOERROR == regexec(&regex, str.c_str(), nmatch, &match_t, 0))
                            {
                                string port = str.substr(match_t.rm_so, match_t.rm_eo - match_t.rm_so);
                                vecport.clear();
                                string strFsep = " ";
                                string_split(port, strFsep, vecport);

                                pattern = string("[0-9]{1,}/(tcp|udp).*");
                                if (REG_NOERROR == regcomp(&regex, pattern.c_str(), REG_EXTENDED | REG_NEWLINE))
                                {
                                    if (REG_NOERROR == regexec(&regex, vecport[0].c_str(), nmatch, &match_t, 0))
                                    {
                                        port_prot_tmp.clear();
                                        string strFsep = "/";
                                        string_split(vecport[0], strFsep, port_prot_tmp);
                                    }
                                }
                                else
                                {
                                    regfree(&regex);
                                    continue;
                                }

                                string temp = port_prot_tmp[0];
                                temp += cThis->m_nmapscanConfig.chFieldSep;
                                temp += port_prot_tmp[1];

                                if(vecport[1].find("|") !=string::npos)
                                {
                                    port_prot_tmp.clear();
                                    string strFsep = "|";
                                    string_split(vecport[1], strFsep, port_prot_tmp);
                                    /*for(int i = 0; i < port_prot_tmp.size(); i++)
                                    {
                                        temp += cThis->m_nmapscanConfig.chFieldSep;
                                        temp += port_prot_tmp[i];
                                    }*/
                                    temp += cThis->m_nmapscanConfig.chFieldSep; //如果有两个状态，取第二个
                                    temp += port_prot_tmp[1];

                                }
                                else
                                {
                                    temp += cThis->m_nmapscanConfig.chFieldSep;
                                    temp += vecport[1];

                                }

                                for(int i = 2; i < vecport.size(); i++)  //从第三个元素开始，第一个已经拆分为端口和协议
                                {
                                    temp += cThis->m_nmapscanConfig.chFieldSep;
                                    temp += vecport[i];
                                }
                               // result_vec.push_back(port_result);
                                result_temp.push_back(temp);
                                //NMAPSCAN_DEBUG_V(string("tcp : ") + temp);
                            }
                            regfree(&regex);
                        }
                    }
                    pclose(fp);


                    // 通过nmap扫描udp设备端口
                    cmd = string("nmap -sU -F ") + strIP;

                    if ((fp = popen(cmd.c_str(), "r")) == NULL)
                    {
                        NMAPSCAN_INFO_S(string("popen udp error !"));
                        return NULL;
                    }
                    while (fgets(fstr, sizeof(fstr), fp) != NULL)  //寻找匹配的开放端口
                    {
                        string str;
                        regex_t regex;
                        string pattern;
                        regmatch_t match_t;
                        size_t nmatch;

                        str = string(fstr);
                        nmatch = 1;

                        pattern = string("[0-9]{1,}/(tcp|udp)[ ]{1,}(open|filtered|closed|unfiltered|\\|){1,}.*");
                        if (REG_NOERROR == regcomp(&regex, pattern.c_str(), REG_EXTENDED | REG_NEWLINE))
                        {
                            if (REG_NOERROR == regexec(&regex, str.c_str(), nmatch, &match_t, 0))
                            {
                                string port = str.substr(match_t.rm_so, match_t.rm_eo - match_t.rm_so);
                                vecport.clear();
                                string strFsep = " ";
                                string_split(port, strFsep, vecport);

                                pattern = string("[0-9]{1,}/(tcp|udp).*");
                                if (REG_NOERROR == regcomp(&regex, pattern.c_str(), REG_EXTENDED | REG_NEWLINE))
                                {
                                    if (REG_NOERROR == regexec(&regex, vecport[0].c_str(), nmatch, &match_t, 0))
                                    {
                                        port_prot_tmp.clear();
                                        string strFsep = "/";
                                        string_split(vecport[0], strFsep, port_prot_tmp);
                                    }
                                }
                                else
                                {
                                    regfree(&regex);
                                    continue;
                                }

                                string temp = port_prot_tmp[0];
                                temp += cThis->m_nmapscanConfig.chFieldSep;
                                temp += port_prot_tmp[1];

                                if(vecport[1].find("|") !=string::npos)
                                {
                                    port_prot_tmp.clear();
                                    string strFsep = "|";
                                    string_split(vecport[1], strFsep, port_prot_tmp);

                                    temp += cThis->m_nmapscanConfig.chFieldSep; //如果有两个状态，取第二个
                                    temp += port_prot_tmp[1];
                                }
                                else
                                {
                                    temp += cThis->m_nmapscanConfig.chFieldSep;
                                    temp += vecport[1];
                                }


                                for(int i = 2; i < vecport.size(); i++)  //从第三个元素开始，第一个已经拆分为端口和协议
                                {
                                    temp += cThis->m_nmapscanConfig.chFieldSep;
                                    temp += vecport[i];
                                }
                                result_temp.push_back(temp);
                               // NMAPSCAN_DEBUG_V(string("udp : ") + temp);
                            }
                            regfree(&regex);
                        }
                    }

                    string resulttemp = strIP;
                    for(int j = 0; j < result_temp.size(); j++)
                    {
                        resulttemp += cThis->m_nmapscanConfig.chRecordSep;
                        resulttemp += result_temp[j];
                    }
                    pclose(fp);


                    if(result_temp.size() > 0)  //避免发送没有结果的ip信息
                    {
                        cThis->m_vecPolicy[i].nResult = resulttemp;
                        cThis->m_vecPolicy[i].nfinished = true;

                    }

					
				}
                if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                    break;

            }

        }
        for(int i = 0; i < (cThis->m_nmapscanConfig.nSleepsec)/5; i++)
        {
            sleep(5);
            if(!cThis->m_vecThreadPool[thread_num].b_state) //收到停止命令, 打断休眠
                break;
        }

	}
	--cThis->m_pthreadnum_alive;
    NMAPSCAN_INFO_S(OnNmapscanHandle Thread exit...);
	
    return NULL;
}

//base64解码
string CNmapscanMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



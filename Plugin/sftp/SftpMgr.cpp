
#include "SftpMgr.h"
#include "utils/bdstring.h"
#include "ssh/bd_sftp_ssh.h"
#include <iostream>
#include <cstdio>
#include <fstream>

using namespace Poco;
using namespace std;
using namespace netprotocol;


CSftp::CSftp(const string &strConfigName)
{    
	m_strConfigName = strConfigName;
    memset(&m_sftpConfig,0,sizeof(m_sftpConfig));
	m_pFuncReportData  = NULL;
	m_pFuncFetchData   = NULL;
	m_bIsRunning       = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

CSftp::~CSftp(void)
{
	m_vecPolicy.clear();
	m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool CSftp::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}

    if( strlen(m_sftpConfig.chLog4File) > 0 ) {
	    if(access(m_sftpConfig.chLog4File, F_OK) != 0) {
            return false;
	    } else {
	        try { 
		        log4cxx::PropertyConfigurator::configure(m_sftpConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			    return false;
	        }
		}
    } else {
        return false;
	}
	
    SFTP_INFO_S( CSftp::Init() successed...);
	printConfig();

    return true;	
}

bool CSftp::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}
UInt32 CSftp::GetModuleId(void) 
{
	return m_sftpConfig.nModuleId;
}
UInt16 CSftp::GetModuleVersion(void) 
{
	return m_sftpConfig.wModuleVersion;
}
string CSftp::GetModuleName(void) 
{
	return m_sftpConfig.chModuleName;
}

//开始（下发)任务
bool CSftp::StartTask(const PModIntfDataType pDataType,const void * pData)
{	
	return true;
}

//停止（取消）任务
bool CSftp::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CSftp::LoadConfig(void)
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
		GET_CONF_ITEM_CHAR(log4configfile,m_sftpConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_sftpConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_sftpConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_sftpConfig.wModuleVersion);
	    GET_CONF_ITEM_CHAR(record_separator,m_sftpConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_sftpConfig.chFieldSep,2);				
	    GET_CONF_ITEM_INT(tasknum_perthread,m_sftpConfig.nTaskNumPerThread);
        GET_CONF_ITEM_INT(readlinenum,m_sftpConfig.nReadLineNum);
		if( m_sftpConfig.nTaskNumPerThread <= 0 ) {
			m_sftpConfig.nTaskNumPerThread = 5;  // 默认每个线程处理5个IP
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

void CSftp::printConfig() {
	SFTP_INFO_S(<=========netservice configure information=========>);
	SFTP_INFO_V(" log4configfile=>    ["+string(m_sftpConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_sftpConfig.nModuleId);
    SFTP_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	SFTP_INFO_V(" modulename=>        ["+string(m_sftpConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_sftpConfig.wModuleVersion);
	SFTP_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	SFTP_INFO_V(" record_separator=>  ["+string(m_sftpConfig.chRecordSep)+string("]"));
	SFTP_INFO_V(" field_separator=>   ["+string(m_sftpConfig.chFieldSep)+string("]"));
	char chTemp[5] = {0};	
	sprintf(chTemp,"%d",m_sftpConfig.nTaskNumPerThread);
	SFTP_INFO_V(" tasknum_perthread=> ["+string(chTemp)+string("]"));
	SFTP_INFO_S(<=========netservice configure information=========>);
}

bool CSftp::Load(void)
{
	return true;
}

//调用方下发数据
bool CSftp::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;   
	m_vecPolicy.clear();		
    vector<string> vec_task_unit;
	string strFsep = m_sftpConfig.chFieldSep;
    string strRsep = m_sftpConfig.chRecordSep;
    string_split(strPolicy,strRsep,vec_task_unit,false);

    SFTP_INFO_S(<=======SFTP PlOCY=======>);
    for( int i = 0; i < vec_task_unit.size(); i++)
    {
        tag_sftp_policy_t tag_policy;
        vector<string> vec_temp;
        SFTP_INFO_V("SFTP POlOCY => " + vec_task_unit[i]);
        //标识ID~IP~用户名~密码~COMMAND(1:get 2:put)~远程目录~本地目录~文件1(目录下全部文件, 则填"*")~文件2...
        string_split(vec_task_unit[i],strFsep,vec_temp);
        tag_policy.vec_options = vec_temp;
        if(vec_temp.size() < 8)
        {
            m_vecPolicy.clear();
            SFTP_ERROR_S(Policy is Unavailable...);
            continue;
        }
        SFTP_INFO_V("<=======strName:" + vec_temp[2] + "===========>");
        tag_policy.strSourceIP = vec_temp[1];
        tag_policy.strUser = vec_temp[2];
        m_vecPolicy.push_back(tag_policy);
    }

    SFTP_INFO_S(<=======SFTP PlOCY=======>);
	
    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_sftpConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间
	
	return true;
}

//调用方获取数据
void * CSftp::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CSftp::FreeData(void * pData)
{
	return;
}

//set callback for report function
void CSftp::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}
//set call back for fetch function
void CSftp::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CSftp::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
     if (m_pFuncReportData){
        return m_pFuncReportData(m_sftpConfig.nModuleId,pDataType,pData,dwLength);
    }   
	 return true;
}
//Fetch Data
const char* CSftp::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CSftp::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	   
	//SetData(NULL,&str,str.length());
    if (! m_bIsRunning ) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        m_vecThreadPool.clear();
        for (int i=0; i < m_pthreadnum; i++) {
			tag_sftp_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnSftpHandle, this) != 0) {
				SFTP_ERROR_S(Create OnSftpHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
            SFTP_INFO_S(Create OnSftpHandle Thread successed!);
		}	
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			SFTP_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		SFTP_INFO_S(Create OnReportHandle Thread successed!);	
	}	

	m_bIsLoaded = true;
	SFTP_INFO_S(CSftp::Start successed!);
	return true;
}

//停止模块
bool CSftp::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}
	SFTP_INFO_S(CSftp::Stop successed!);
    return true;
}

void *CSftp::OnFetchHandle(void *arg) {
	return NULL;
}

// 上报任务处理结果
void *CSftp::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CSftp *cThis = reinterpret_cast<CSftp*>(arg);

    if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
	while(true) {
        vector<tag_sftp_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {
            /*
			modintf_datatype_t temp;
			temp.nDataType	   = MODULE_DATA_PERFORMANCE;
			temp.nMainCategory = MODULE_DATA_PERFORMANCE_SFTP;	
			int nChildTaskNum = iter_temp->m_mapFinished.size();
            for(int i=0 ; i < nChildTaskNum; i++) {
				if( iter_temp->m_mapFinished[i] ) { // 任务完成	
					if(!cThis->ReportData(&temp,(void *)&(iter_temp->m_mapResult[i]),iter_temp->m_mapResult[i].length()) ) {
                        SFTP_ERROR_S(Report data failed!);
					} else {
						SFTP_DEBUG_S(Report data success!);
                    }
                      
                    cThis->Stop();
					iter_temp->m_mapFinished[i] = false;  // 成功或失败下次都重新执行任务
				}	
            }*/
            if( iter_temp->bSendFalg ) { // 可以发送日志
                vector<string>::iterator iter_log = iter_temp->vec_log.begin();
                modintf_datatype_t temp;
                temp.nDataType     = MODULE_DATA_LOG;
                temp.nMainCategory = MODULE_DATA_LOG_SFTP;

                time_t t = time(NULL);
                struct tm now;
                localtime_r(&t, &now);
                char chLogDate[22] = {0};
                sprintf(chLogDate,"%04d-%02d-%02d %02d:%02d:%02d",now.tm_year+1900
                                                                 ,now.tm_mon+1
                                                                 ,now.tm_mday
                                                                 ,now.tm_hour
                                                                 ,now.tm_min
                                                                 ,now.tm_sec);
                /*按照和syslog输出的格式拼接filelog日志
                 *yyyy-mm-dd hh:mi:ss~IP~usr~notice~filelog~CONTENT
                 */
                string strTemp    = string(chLogDate)       + cThis->m_sftpConfig.chFieldSep
                                  + iter_temp->strSourceIP	+ cThis->m_sftpConfig.chFieldSep
                                  + iter_temp->strUser      + cThis->m_sftpConfig.chFieldSep
                                  + "notice"			    + cThis->m_sftpConfig.chFieldSep
                                  + "sftp"                  + cThis->m_sftpConfig.chFieldSep;
                for(; iter_log != iter_temp->vec_log.end(); iter_log++) {
                    string strResult = strTemp + *iter_log;
                    if(! cThis->ReportData(&temp,(void *)&strResult,strResult.length()) ) {
                        SFTP_DEBUG_S(Report data failed!);
                    } else {
                        static long int nums = 0;
                        if(++nums >= 10000 || iter_log == iter_temp->vec_log.end()-1)
                        {
                            char ch_num[10] = {0};
                            sprintf(ch_num,"[%ld]",nums);
                            SFTP_DEBUG_V("Report data success ! " + string(ch_num));
                            nums = 0;
                        }
                    }
                }
                iter_temp->vec_log.clear();    // 清空已发送日志
                iter_temp->bSendFalg = false;  // 重置发送标志
            }
        }		
		if(cThis->m_pthreadnum_alive == 0 ) break; //处理线程池全部退出后则退出该线程	
		if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
	cThis->m_bIsRunning = false;	
	cThis->m_vecThreadPool.clear();
 	SFTP_INFO_S(OnReportHandle Thread exit...);
    return NULL;

}

void CSftp::ReadFileLog(void *arg, int thread_num, string fileName)
{
    CSftp *cThis = reinterpret_cast<CSftp*>(arg);
    ifstream inFile(fileName.c_str());
    string strOneLine;

    while (getline(inFile, strOneLine))
    {
        if (strOneLine.empty()) continue;

        while (cThis->m_vecPolicy[thread_num].bSendFalg) {
            usleep(10000);
        }

        cThis->m_vecPolicy[thread_num].vec_log.push_back(strOneLine);
        if ( cThis->m_vecPolicy[thread_num].vec_log.size() >= cThis->m_sftpConfig.nReadLineNum){
            cThis->m_vecPolicy[thread_num].bSendFalg = true;
        }
    }

    cThis->m_vecPolicy[thread_num].bSendFalg = true;
    inFile.close();
}

void* CSftp::OnSftpHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CSftp *cThis = reinterpret_cast<CSftp*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();
    string result = "";
    int nFailure = 0;
    vector<string> log_contents;                       //存储日志文件列表

	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning) {		
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) { 
                if(cThis->m_vecPolicy[i].m_mapFinished[0] == false)
                {
                    //string str = "标识ID~IP~用户名~密码~COMMAND(1:get 2:put)~远程目录~本地目录~文件1(目录下全部文件, 则填"*")~文件2"
                    vector<string> v_options_tmp = cThis->m_vecPolicy[i].vec_options;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除标识结点

                    vector<string>::iterator iter = v_options_tmp.begin();
                    string ip = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除ip结点
                    iter = v_options_tmp.begin();
                    string username = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除用户名结点
                    iter = v_options_tmp.begin();
                    string password = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除密码结点
                    iter = v_options_tmp.begin();
                    string command = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除command结点
                    iter = v_options_tmp.begin();
                    string remotedir = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除远程目录结点
                    iter = v_options_tmp.begin();
                    string localdir = *iter;
                    v_options_tmp.erase(v_options_tmp.begin()); //删除本地目录结点

                    if(remotedir.length() != remotedir.rfind('/')+1)
                        remotedir += '/';
                    if(localdir.length() != localdir.rfind('/')+1)
                        localdir += '/';

                    /* 设置默认result值 */
                    char tmp[8];
                    sprintf(tmp, "%ld", v_options_tmp.size());
                    result = tmp;

                    bd_sftp_libssh2 sftphd;
                    if(!sftphd.sftpInit(ip, 22, username, password)) {
                        SFTP_ERROR_V("sftpInit failed: " + sftphd.getLastError());
                        goto REPORT;
                    }
                    if(command.compare("1") == 0)
                    {
                        for(iter = v_options_tmp.begin(); iter != v_options_tmp.end(); iter++)
                        {
                            if((*iter).find("*") != string::npos)
                            {
                                vector<string> vec_filelist;
                                if(sftphd.getSftpFList(remotedir, vec_filelist))
                                {
                                    SFTP_ERROR_V("Get remote filelist failed: " + sftphd.getLastError());
                                    continue;
                                }
                                vector<string>::iterator it_list = vec_filelist.begin();
                                for(; it_list != vec_filelist.end(); it_list++)
                                {
                                    string file_type = it_list->substr(it_list->find_last_of('.') + 1);
                                    if( file_type == "txt" || file_type == "hex" || file_type == "log" )
                                    {
                                        if(sftphd.download(localdir + *it_list, remotedir + *it_list))
                                        {
                                            nFailure ++;
                                            SFTP_ERROR_V("download failed for " + *it_list + sftphd.getLastError());
                                        }

                                        if(sftphd.rmSftpFileName(remotedir + *it_list))
                                        {
                                            nFailure ++;
                                            SFTP_ERROR_V("rm failed for " + *it_list + sftphd.getLastError());
                                        }

                                        /*
                                        //to read log content send AnalyzeServer
                                        std::ifstream ifile((localdir + *it_list).c_str());
                                        std::string line;

                                        while (std::getline(ifile, line)){
                                            log_contents.push_back(line);
                                        }
                                        ifile.close();*/
                                        //read file log
                                        cThis->ReadFileLog(arg, i, (localdir + *it_list).c_str());
                                    }
                                }
                            } else
                            {
                                string file_type = iter->substr(iter->find_last_of('.') + 1);
                                if(file_type == "txt")
                                {
                                    if(sftphd.download(localdir + *iter, remotedir + *iter))
                                    {
                                        nFailure ++;
                                        SFTP_ERROR_V("download failed for " + *iter + sftphd.getLastError());
                                    }

                                    if(sftphd.rmSftpFileName(remotedir + *iter))
                                    {
                                        nFailure ++;
                                        SFTP_ERROR_V("rm failed for " + *iter + sftphd.getLastError());
                                    }
                                    /*
                                    //to read log content send AnalyzeServer
                                    std::ifstream ifile((localdir + *iter).c_str());
                                    std::string line;

                                    while (std::getline(ifile, line)){
                                        log_contents.push_back(line);
                                    }
                                    ifile.close();*/
                                    cThis->ReadFileLog(arg, i, (localdir + *iter).c_str());
                                }
                            }
                        } //end for(iter...)
                    } else if(command.compare("2") == 0)
                    {
                        for(iter = v_options_tmp.begin(); iter != v_options_tmp.end(); iter++)
                        {
                            if((*iter).find("*") != string::npos)
                            {
                                vector<string> vec_filelist;
                                if(sftphd.getLocalFList(localdir, vec_filelist))
                                {
                                    SFTP_ERROR_V("Get remote filelist failed: " + sftphd.getLastError());
                                    continue;
                                }
                                vector<string>::iterator it_list = vec_filelist.begin();
                                for(; it_list != vec_filelist.end(); it_list++)
                                {
                                    if(sftphd.upload(localdir + *it_list, remotedir + *it_list))
                                    {
                                        nFailure ++;
                                        SFTP_ERROR_V("download failed for " + *it_list);
                                    }
                                }
                            } else
                            {
                                if(sftphd.upload(localdir + *iter, remotedir + *iter))
                                {
                                    nFailure ++;
                                    SFTP_ERROR_V("upload failed for " + *iter);
                                }
                            }
                        }
                    } else
                    {
                        SFTP_ERROR_S(Unkown command...);
                        goto REPORT;
                    }

                    sprintf(tmp, "%d", nFailure); //修正result
                    result = tmp;

                REPORT:
                    sftphd.sftpRelease();
                    /*
                    //标识ID~IP~COMMAND~结果标识（0：成功  非0：未能成功传输文件数）
                    string SrcReport = cThis->m_vecPolicy[i].vec_options[0];
                    SrcReport += cThis->m_sftpConfig.chFieldSep;
                    SrcReport += cThis->m_vecPolicy[i].vec_options[1];
                    SrcReport += cThis->m_sftpConfig.chFieldSep;
                    SrcReport += cThis->m_vecPolicy[i].vec_options[2];
                    SrcReport += cThis->m_sftpConfig.chFieldSep;

                    SrcReport += result;

                    SrcReport += cThis->m_sftpConfig.chFieldSep;

                    for (std::vector<string>::iterator log = log_contents.begin(); ; ){
                        SrcReport += *log;
                        log++;

                        if (log == log_contents.end()) break;

                        SrcReport += cThis->m_sftpConfig.chRecordSep;
                    }

                    cThis->m_vecPolicy[i].m_mapResult[0] = SrcReport;
                    cThis->m_vecPolicy[i].m_mapFinished[0] = true;
                    SFTP_DEBUG_V("sftp result:"+SrcReport);*/
                    sleep(3);
                } //end if(cThis->m_vecPolicy[i].m_mapFinished[0] == false)
            }
        } //if(cThis->m_bIsRunning)
    }
    while(true)
    {
        if(!(cThis->m_vecThreadPool[thread_num].b_state))
        {
            --cThis->m_pthreadnum_alive;
            break;
        }
    }
 	SFTP_INFO_S(OnSftpHandle Thread exit...);
    return NULL;
}

//base64解码
string CSftp::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}



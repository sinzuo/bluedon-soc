
#include "FileLogMgr.h"
#include "utils/bdstring.h"
#include  "unistd.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>

#define WRITE_RATE  1000

using namespace Poco;
using namespace std;

CFileLogMgr::CFileLogMgr(const string &strConfigName)
{
	m_strConfigName = strConfigName;
    memset(&m_fileLogConfig,0,sizeof(m_fileLogConfig));
	
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;
    m_pthreadnum       = 0;
	m_pthreadnum_alive = 0;
	m_bIsLoaded = false;

	m_vecPolicy.clear();
	m_vecThreadPool.clear();

}

CFileLogMgr::~CFileLogMgr(void)
{
}

//初始化数据，先于Start被调用
bool CFileLogMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    
    if(!LoadConfig()) {
		return false;
	}
    if( strlen(m_fileLogConfig.chLog4File) > 0 ) {
	    if(access(m_fileLogConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
			 try { 
		        log4cxx::PropertyConfigurator::configure(m_fileLogConfig.chLog4File);
		        //g_logger = Logger::getLogger("LOG4_PROXY"); 
	        } catch (Exception &) { 
			return false;
	        }
		}
    }
	else {
        return false;
	}

	printConfig();

    FILELOG_INFO_S( CFileLogMgr::Init() successed...);

	return true;	
}

//检查模块是否处于运行状态
bool CFileLogMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}

UInt32 CFileLogMgr::GetModuleId(void) 
{
	return m_fileLogConfig.nModuleId;
}

UInt16 CFileLogMgr::GetModuleVersion(void) 
{
	return m_fileLogConfig.wModuleVersion;
}

string CFileLogMgr::GetModuleName(void) 
{
	return m_fileLogConfig.chModuleName;
}

//开始（下发)任务
bool CFileLogMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	return true;
}

//停止（取消）任务
bool CFileLogMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CFileLogMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
		cerr << "FileLog LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "FileLog LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

	try {
		GET_CONF_ITEM_CHAR(log4configfile,m_fileLogConfig.chLog4File,100);
		GET_CONF_ITEM_INT(moduleid,m_fileLogConfig.nModuleId);	 
		GET_CONF_ITEM_CHAR(modulename,m_fileLogConfig.chModuleName,20);
		GET_CONF_ITEM_INT(moduleversion,m_fileLogConfig.wModuleVersion);		
	    GET_CONF_ITEM_CHAR(record_separator,m_fileLogConfig.chRecordSep,2);
	    GET_CONF_ITEM_CHAR(field_separator,m_fileLogConfig.chFieldSep,2);				
		GET_CONF_ITEM_INT(readlinenum,m_fileLogConfig.nReadLineNum);
		if( m_fileLogConfig.nReadLineNum <= 0) {
			m_fileLogConfig.nReadLineNum = 100;     // 默认一次读取 100行
		}
		GET_CONF_ITEM_INT(tasknum_perthread,m_fileLogConfig.nTaskNumPerThread);	
		if( m_fileLogConfig.nTaskNumPerThread <= 0 ) {
		    m_fileLogConfig.nTaskNumPerThread = 1;  // 默认每个线程处理1个目录
		}
        GET_CONF_ITEM_BOOL(largelog,m_fileLogConfig.bLargeLog);
    } catch (NotFoundException& exc ) {
		cerr << "FileLog LoadConfig:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "FileLog LoadConfig:" << exc.displayText() << endl;	
		return false;
	}

    return true;
}


void CFileLogMgr::printConfig() {
	FILELOG_INFO_S(<========filelog configure information========>);
	FILELOG_INFO_V(" log4configfile=>    ["+string(m_fileLogConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId,"%d",m_fileLogConfig.nModuleId);
    FILELOG_INFO_V(" moduleid=>          ["+string(chModuleId)+string("]"));
	FILELOG_INFO_V(" modulename=>        ["+string(m_fileLogConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion,"%d",m_fileLogConfig.wModuleVersion);
	FILELOG_INFO_V(" moduleversion=>     ["+string(chVersion)+string("]"));
	FILELOG_INFO_V(" record_separator=>  ["+string(m_fileLogConfig.chRecordSep)+string("]"));
	FILELOG_INFO_V(" field_separator=>   ["+string(m_fileLogConfig.chFieldSep)+string("]"));	
    char chReadLineNum[5] = {0};
	sprintf(chReadLineNum,"%d",m_fileLogConfig.nReadLineNum);
	FILELOG_INFO_V(" readlinenum=>       ["+string(chReadLineNum)+string("]"));
	char chTaskNumPerThread[5] = {0};	
	sprintf(chTaskNumPerThread,"%d",m_fileLogConfig.nTaskNumPerThread);
	FILELOG_INFO_V(" tasknum_perthread=> ["+string(chTaskNumPerThread)+string("]"));
	
	FILELOG_INFO_S(<========filelog configure information========>);
}


bool CFileLogMgr::Load(void)
{
    return true;
}

//调用方下发数据
bool CFileLogMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	FILELOG_INFO_S(<========FILELOG PlOCY========>);
	FILELOG_INFO_V(" FILELOG PlOCY =>" + strPolicy);
	FILELOG_INFO_S(<========FILELOG PlOCY========>);
	m_vecPolicy.clear();	
	vector<string> vecDir;
	string strRsep = m_fileLogConfig.chRecordSep;
	string_split(strPolicy,strRsep,vecDir);
	vector<string>::iterator iter = vecDir.begin();
	for(;iter != vecDir.end(); iter++) {
		tag_filelog_policy_t temp;
        if( (temp.dirPtr = opendir(iter->c_str())) == NULL){ //判断目录是否存在
			FILELOG_ERROR_V( "open directory failed => " + *iter);
			continue;
		}
		string strPathTmp;
        if (iter->length() == iter->rfind("/")+1) {
            strPathTmp = iter->substr(0,iter->rfind("/")); // 临时去掉最后一位 /
		} else {
		    strPathTmp = *iter;
		}
        string strDirName = strPathTmp.substr(strPathTmp.rfind("/")+1);
		string strFsep = m_fileLogConfig.chFieldSep;
		vector<string> vecDirName;
		// IP~APPNAME
		string_split(strDirName,strFsep,vecDirName);	
		if(vecDirName.size() != 2) return false;

		if (iter->length() != iter->rfind("/")+1) {
            *iter += "/";   // 为目录最后一位补 /
		}
		temp.strDirPath  = *iter;
		temp.strSourceIP = vecDirName[0];
		temp.strAppName  = vecDirName[1];
        closedir(temp.dirPtr); //关闭目录描述符指针
        temp.dirPtr = NULL;
        temp.vec_log.reserve(m_fileLogConfig.nReadLineNum);
		m_vecPolicy.push_back(temp);	
	} 

    m_pthreadnum = (int)ceil((double)m_vecPolicy.size()/m_fileLogConfig.nTaskNumPerThread);
	m_vecThreadPool.reserve(m_pthreadnum);     // 提前分配线程池空间
	return true;
}

//调用方获取数据
void * CFileLogMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CFileLogMgr::FreeData(void * pData)
{
	
}

//set callback for report function
void CFileLogMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}

//set call back for fetch function
void CFileLogMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CFileLogMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    if (m_pFuncReportData){
        bool ret = m_pFuncReportData(m_fileLogConfig.nModuleId,pDataType,pData,dwLength);
        while(!ret && errno == EAGAIN){
            //FILELOG_DEBUG_V(string("m_pFuncReportData Failed, ") + strerror(EAGAIN));
            usleep(1000);
            ret = m_pFuncReportData(m_fileLogConfig.nModuleId,pDataType,pData,dwLength);
        }
        return ret;
    }
    return false;
}

//Fetch Data
const char* CFileLogMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CFileLogMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);

    /* for test.....
     string test = "/root/zhangjie/soc5.0/dir/172.16.33.3~mysql|/root/zhangjie/soc5.0/dir/172.16.33.1~tomcat";
	 SetData(NULL,&test,0);*/   
    if (! m_bIsRunning )
    {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
		for (int i=0; i<m_pthreadnum; i++) {
			tag_filelog_threadpool_t temp;
			memset(&temp, 0, sizeof(temp));
			temp.t_num = i;
			m_vecThreadPool.push_back(temp);
            if (pthread_create(&(m_vecThreadPool[i].t_id), &pa, OnFilelogHandle, this) != 0) {
				FILELOG_ERROR_S(Create OnFilelogHandle Thread failed!);
				return false;
			}
			usleep(10000); // 10毫秒，启动线程需要一些执行时间
			m_pthreadnum_alive++;
			FILELOG_INFO_S(Create OnFilelogHandle Thread successed!);
		}
		
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
			FILELOG_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
		usleep(1000); // 1毫秒 
		FILELOG_INFO_S(Create OnReportHandle Thread successed!);
	}	
	m_bIsLoaded = true;
	FILELOG_INFO_S(CFileLogMgr::Start successed!);

    return true;
	
}

//停止模块
bool CFileLogMgr::Stop(void)
{

	FastMutex::ScopedLock lock(m_ModMutex);
	m_bIsLoaded = false;

	for(int i=0; i< m_pthreadnum; i++) {
		m_vecThreadPool[i].b_state = false;
	}
	FILELOG_INFO_S(CFileLogMgr::Stop successed!);
    return true;
	
}

void CFileLogMgr::GetFileName(tag_filelog_policy_p policyPtr, list<tag_filelog_info_t> &filenamelist) //TODO, 感觉不完善
{
	struct dirent *file; 
    policyPtr->dirPtr = opendir(policyPtr->strDirPath.c_str()); //实时监测目录下文件列表
	while((file = readdir(policyPtr->dirPtr)) != NULL) //读取待处理文件
	{
		struct stat st; 	 
		tag_filelog_info_t tagFileInfoTemp;
		if(strncmp(file->d_name, ".", 1) == 0)  continue; // 过滤 ".*"
        //string strPath = policyPtr->strDirPath.substr(0, policyPtr->strDirPath.rfind("/")+1);
		tagFileInfoTemp.strFilename = policyPtr->strDirPath + string(file->d_name);
		//文件状态获取失败，或者为目录则 continue
		if(stat(tagFileInfoTemp.strFilename.c_str(), &st) < 0 || S_ISDIR(st.st_mode)) continue;
		tagFileInfoTemp.mtime = st.st_mtime; //最后一次修改该文件的时间  
		filenamelist.push_back(tagFileInfoTemp);
	}
    closedir(policyPtr->dirPtr);
    policyPtr->dirPtr = NULL;
	//按文件修改时间排序
	if(filenamelist.size() > 0)
		filenamelist.sort(); 
}

void CFileLogMgr::Backup(const string &strFileName)  //备份文件
{
    if( strFileName.length() == 0) return;

	time_t t = time(NULL);
	struct tm now;
	localtime_r(&t, &now);
	char chDate[11] = {0};
	sprintf(chDate,"%04d-%02d-%02d",now.tm_year+1900
								   ,now.tm_mon+1
								   ,now.tm_mday);					

	string strBakPath = strFileName.substr(0, strFileName.rfind("/")+1);
	strBakPath += string(chDate);
	strBakPath +=  "/";
    if(opendir(strBakPath.c_str()) == NULL) {
		FILELOG_INFO_V("start to create directory :" + strBakPath);
		if (mkdir(strBakPath.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH)<0) {
			FILELOG_ERROR_S(create directory failed!);
			return;
		}
	}
	strBakPath += strFileName.substr(strFileName.rfind("/")+1);
	FILELOG_DEBUG_V("rename " + strFileName + " to " + strBakPath);

    if( rename(strFileName.c_str(), strBakPath.c_str()) != 0) //rename()移动或重命名
    {
		FILELOG_ERROR_S(rename failed!);
    }
}

bool CFileLogMgr::InitPosFile(const tag_filelog_policy_t &policy, list<tag_filelog_info_t> &filenamelist)
{
    string strMarkPath = policy.strDirPath + "MarkPos";

    if(access(strMarkPath.c_str(), F_OK) != 0) //检查目录是否存在
    {
        FILELOG_INFO_V("start to create directory :" + strMarkPath);
        if (mkdir(strMarkPath.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH) < 0)
        {
            FILELOG_ERROR_S(create position directory failed!);
            return false;
        }
    }

    strMarkPath += "/posList.pos";
    if(access(strMarkPath.c_str(), F_OK) != 0) //检查文件是否存在
    {
        FILELOG_INFO_V("start to create file :" + strMarkPath);
        ofstream outFile(strMarkPath.c_str()); //创建新文件
        if (!outFile)
        {
            FILELOG_ERROR_S(create position file failed!);
            return false;
        }
        outFile.close();
    }

    fstream ioFile(strMarkPath.c_str(), ios::in|ios::out);
    if (!ioFile)
    {
        FILELOG_ERROR_S(open position file failed!);
        return false;
    }
    list<tag_filelog_info_t>::iterator iterTmp = filenamelist.begin();

    streamoff markPos;
    long long posCurrent, fileSize;
    for(;iterTmp != filenamelist.end(); iterTmp++) //初始化文件列表
    {
        if(ReadPos(ioFile, markPos, iterTmp->strFilename, posCurrent, fileSize)) continue;
        else MarkPos(ioFile, -1, iterTmp->strFilename, 0, 0);
    }
    ioFile.close();

    return true;
}

void CFileLogMgr::MarkPos(fstream &fPosFile, streamoff markPos, const string &strFileName, long long posCurrent, long long fileSize)  //标记位置
{
    if( strFileName.length() == 0) return;

    string strName = strFileName.substr(strFileName.rfind("/")+1);

    char chPosCurrent[40];
    char chFileSize[40];
    sprintf(chPosCurrent, "%lld", (long long)posCurrent);
    sprintf(chFileSize, "%lld", (long long)fileSize);

    string strFileInfo;
    strFileInfo += chPosCurrent;
    strFileInfo += m_fileLogConfig.chFieldSep;
    strFileInfo += chFileSize;

    fPosFile.clear(); //重置状态标志符

    if(markPos < 0)
    {
        FILELOG_INFO_V("Begin adding new record: " + strName);
        fPosFile.seekp(0, ios::end);
        fPosFile << strName << '\n';
        fPosFile << left << setw(80) << strFileInfo << '\n';
    }
    else
    {
        fPosFile.seekp(markPos);
        fPosFile << left << setw(80) << strFileInfo;
    }
}

bool CFileLogMgr::ReadPos(fstream &fPosFile, streamoff &markPos, const string &strFileName, long long &posCurrent, long long &fileSize)
{
    if( strFileName.length() == 0) return false;
    string strFileInfo;

    string strName = strFileName.substr(strFileName.rfind("/")+1);

    string strOneLine;
    streamoff offMarkPos = 0;
    fPosFile.seekg(0); //将文件指针重新指向头部
    while(getline(fPosFile, strOneLine))
    {
        offMarkPos = fPosFile.tellg();
        if(strOneLine.compare(strName) == 0)
        {
            markPos = offMarkPos;
            getline(fPosFile, strFileInfo);

            vector<string> vecFileInfo;
            string strFsep = m_fileLogConfig.chFieldSep;
            string_split(strFileInfo,strFsep,vecFileInfo);
            if(vecFileInfo.size() < 2) continue;
            posCurrent = atoll(vecFileInfo[0].c_str());
            fileSize = atoll(vecFileInfo[1].c_str());
            return true;
        }
    }

    fPosFile.clear(); //清除eof状态
    markPos = -1;
    posCurrent = 0;
    fileSize = 0;

    return false;
}


void *CFileLogMgr::OnFetchHandle(void *arg) {
    return NULL;
}

//DONE
void *CFileLogMgr::OnReportHandle(void *arg)
{
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
    CFileLogMgr *cThis = reinterpret_cast<CFileLogMgr*>(arg);

	while(true) {
		if ( cThis->m_pthreadnum == cThis->m_pthreadnum_alive ) {
			cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
		}
        vector<tag_filelog_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();
        for( ;iter_temp != cThis->m_vecPolicy.end(); iter_temp++) {	
			if( iter_temp->bSendFalg ) { // 可以发送日志
                vector<string>::iterator iter_log = iter_temp->vec_log.begin();
				modintf_datatype_t temp;
				temp.nDataType     = MODULE_DATA_LOG;
				temp.nMainCategory = MODULE_DATA_LOG_FILELOG;

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
				string strTemp    = string(chLogDate)       + cThis->m_fileLogConfig.chFieldSep  
								  + iter_temp->strSourceIP	+ cThis->m_fileLogConfig.chFieldSep  
								  + "user"			        + cThis->m_fileLogConfig.chFieldSep    
								  + "notice"			    + cThis->m_fileLogConfig.chFieldSep    
								  + iter_temp->strAppName   + cThis->m_fileLogConfig.chFieldSep;  
				for(; iter_log != iter_temp->vec_log.end(); iter_log++) {
					string strResult = strTemp + *iter_log;
					if(! cThis->ReportData(&temp,(void *)&strResult,strResult.length()) ) {
						FILELOG_DEBUG_S(Report data failed!);
					} else {
                        static long int nums = 0;
                        if(++nums >= 10000 || iter_log == iter_temp->vec_log.end()-1)
                        {
                            char ch_num[10] = {0};
                            sprintf(ch_num,"[%ld]",nums);
                            FILELOG_DEBUG_V("Report data success ! " + string(ch_num));
                            nums = 0;
                        }
					}
				}
				iter_temp->vec_log.clear();    // 清空已发送日志
				iter_temp->bSendFalg = false;  // 重置发送标志 
			}
        }	
		if(cThis->m_pthreadnum_alive == 0 ) {
			break; //线程池全部退出后则退出该线程
		}
		if(cThis->m_bIsRunning) sleep(1); // 不用一直扫描
    }
	cThis->m_bIsRunning = false;	
	cThis->m_vecThreadPool.clear();
 	FILELOG_INFO_S(OnReportHandle Thread exit...);
	
    return NULL;
}

//DONE
 void* CFileLogMgr::OnFilelogHandle(void *arg) {
    //if (pthread_detach(pthread_self()) ==0 ) PING_INFO_S(start thread with detach state...);
	CFileLogMgr *cThis = reinterpret_cast<CFileLogMgr*>(arg);

	int thread_num = cThis->m_vecThreadPool.size() - 1;  // 当前线程num号
	cThis->m_vecThreadPool[thread_num].b_state = true;   // 线程状态修改为启动
	int nPolocyNum = cThis->m_vecPolicy.size();

	while( cThis->m_vecThreadPool[thread_num].b_state) {
		if(cThis->m_bIsRunning ) {				
			for(int i=thread_num; i< nPolocyNum; i += cThis->m_pthreadnum) {
				list<tag_filelog_info_t> lis_file;  // 存放待处理日志文件名	 				
                cThis->GetFileName(&cThis->m_vecPolicy[i], lis_file); // 获取待采集的日志文件并排序

                string posFilePath;
                streamoff markPos = 0;
                if(cThis->m_fileLogConfig.bLargeLog){
                    if(!cThis->InitPosFile(cThis->m_vecPolicy[i], lis_file))
                    {
                        FILELOG_ERROR_S(Init position file failed!);
                        continue;
                    }
                    posFilePath = cThis->m_vecPolicy[i].strDirPath + "MarkPos/posList.pos";
                } else
                    posFilePath = "/tmp/posList.pos";
                fstream fPosFile(posFilePath.c_str(), ios::in|ios::out);                

				list<tag_filelog_info_t>::iterator iterTmp = lis_file.begin();
                for(;iterTmp != lis_file.end(); iterTmp++)
                {
					FILELOG_INFO_V("ready to open file : "+ iterTmp->strFilename);
					ifstream inFile(iterTmp->strFilename.c_str());
					if(!inFile) {
						FILELOG_ERROR_V("open file " + iterTmp->strFilename + string(" failed!"));
						cThis->Backup(iterTmp->strFilename);
						continue;
					}                   

                    long long posCurrent, fileSize;
                    streamoff posNext;
                    int nCount = 0;
                    struct stat st;
                    if(cThis->m_fileLogConfig.bLargeLog){
                        if(stat(iterTmp->strFilename.c_str(), &st) < 0) //获取文件信息
                        {
                            FILELOG_ERROR_V("Get file [" + iterTmp->strFilename + string("] stat failed!"));
                            inFile.close();
                            continue;
                        }
                        if(cThis->ReadPos(fPosFile, markPos, iterTmp->strFilename, posCurrent, fileSize)) //读取标记位
                        {
                            if(fileSize > st.st_size) //判断文件是否曾被截断
                            {
                                FILELOG_INFO_V(iterTmp->strFilename + " may be truncated once!");
                                posCurrent = 0;
                            }else if(posCurrent == st.st_size) //判断是否已发完
                            {
                                FILELOG_INFO_V(iterTmp->strFilename + " have been sent already!");
                                inFile.close();
                                continue;
                            }
                        }
                        inFile.seekg(posCurrent); //跳转到记录位置
                        posNext = posCurrent;
                    }

                    string strOneLine;                   
                    while(getline(inFile, strOneLine))
                    {
                        if(cThis->m_fileLogConfig.bLargeLog){
                            posCurrent = posNext;
                            posNext = inFile.tellg();
                            if(posNext < 0 || posNext > st.st_size) posNext = (streamoff)st.st_size;
                            if(nCount >= WRITE_RATE) //隔几行写入一次
                            {
                                cThis->MarkPos(fPosFile, markPos, iterTmp->strFilename, posCurrent, st.st_size); //记录位置
                                nCount = 0;
                            }
                            nCount ++;
                        }

						if(strOneLine.empty()) continue;
						while(cThis->m_vecPolicy[i].bSendFalg) {
                            usleep(10000);   //等待日志发送完毕再继续读文件
						}
						cThis->m_vecPolicy[i].vec_log.push_back(strOneLine);
                        if ( cThis->m_vecPolicy[i].vec_log.size() >= cThis->m_fileLogConfig.nReadLineNum){
                            cThis->m_vecPolicy[i].bSendFalg = true;
						}
					}

                    cThis->m_vecPolicy[i].bSendFalg = true;
                    inFile.close();

                    if(cThis->m_fileLogConfig.bLargeLog){
                        posCurrent = posNext;
                        cThis->MarkPos(fPosFile, markPos, iterTmp->strFilename, posCurrent, posNext); //记录位置
                        fPosFile.flush(); //刷入文件
                    } else {
                        cThis->Backup(iterTmp->strFilename);
                    }
                }
                fPosFile.close();
			}
            sleep(60);
		}
	}
	--cThis->m_pthreadnum_alive;
 	FILELOG_INFO_S(OnFilelogHandle Thread exit...);

    return NULL;

}




//base64解码
string CFileLogMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}


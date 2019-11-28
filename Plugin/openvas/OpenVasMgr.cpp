#include "OpenVasMgr.h"
#include "utils/bdstring.h"
#include "task/bd_task.h"
#include <iostream>
#include <cstdio>

using namespace Poco;
using namespace std;

COpenVasMgr::COpenVasMgr(const string& strConfigName) {
    m_strConfigName = strConfigName;
    memset(&m_OpenVasConfig, 0, sizeof(m_OpenVasConfig));
    m_pFuncReportData  = NULL;
    m_pFuncFetchData   = NULL;
    m_bIsRunning       = false;
    m_pthreadnum       = 0;
    m_pthreadnum_alive = 0;
    m_bTimeOut         = false;
    m_bTimeOutStatus  = false;

    //TODO
    m_vecPolicy.clear();
    m_listPolicy.clear();
    m_listReport.clear();
    m_vecThreadPool.clear();
}

COpenVasMgr::~COpenVasMgr(void) {
    m_vecPolicy.clear();
    m_vecThreadPool.clear();
}

//初始化数据，先于Start被调用
bool COpenVasMgr::Init(void) {
    if (access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
    }

    if (!LoadConfig()) {
        return false;
    }

    if (strlen(m_OpenVasConfig.chLog4File) > 0) {
        if (access(m_OpenVasConfig.chLog4File, F_OK) != 0) {
            return false;

        } else {
            try {
                log4cxx::PropertyConfigurator::configure(m_OpenVasConfig.chLog4File);
                //g_logger = Logger::getLogger("LOG4_PROXY");

            } catch (Exception& e) {
                return false;
            }
        }

    } else {
        return false;
    }

    log_info(" COpenVasMgr::Init() successed...");

    printConfig();

    return true;
}

bool COpenVasMgr::IsRunning(void) {
    FastMutex::ScopedLock lock(m_ModMutex);
    return m_bIsRunning;
}
UInt32 COpenVasMgr::GetModuleId(void) {
    return m_OpenVasConfig.nModuleId;
}
UInt16 COpenVasMgr::GetModuleVersion(void) {
    return m_OpenVasConfig.wModuleVersion;
}
string COpenVasMgr::GetModuleName(void) {
    return m_OpenVasConfig.chModuleName;
}

//开始（下发)任务
bool COpenVasMgr::StartTask(const PModIntfDataType pDataType, const void* pData) {
    FastMutex::ScopedLock lock(m_ModMutex);
    string strPolicy = *(string*)pData;

    vector<string> vec_task_unit;
    string strRsep = m_OpenVasConfig.chRecordSep;
    string_split(strPolicy, strRsep, vec_task_unit);

    log_info("<=======OpenVas PlOCY Begin=======>");

    int nNum = vec_task_unit.size();  // 策略数量

    for (int i = 0; i < nNum; i++) {
        log_info(" OpenVas POLOCY =>%s", vec_task_unit[i].c_str());
        tag_OpenVas_policy_t tag_policy;
        vector<string> vec_temp;
        string strFsep = m_OpenVasConfig.chFieldSep;
        // taskid~cmd~host
        string_split(vec_task_unit[i], strFsep, vec_temp, false);

        tag_policy.vec_options = vec_temp;

        tag_policy.m_Finished = false;
        tag_policy.m_Result.clear();
        m_listPolicy.push_back(tag_policy);
    }

    log_info("policy list size is :%d", m_listPolicy.size());
    log_info("<=======OpenVas PlOCY end=======>");

    return true;
}

//停止（取消）任务
bool COpenVasMgr::StopTask(const PModIntfDataType pDataType, const void* pData) {
    return true;
}

bool COpenVasMgr::LoadConfig(void) {
    AutoPtr<Poco::Util::IniFileConfiguration> pConf = NULL;

    try {
        pConf = new Poco::Util::IniFileConfiguration(m_strConfigName);
        sleep(1);

    } catch (NotFoundException& exc) {
        cerr << "OpenVas LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;

    } catch (SyntaxException& exc) {
        cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
        return false;

    } catch (Exception& exc) {
        cerr << "OpenVas LoadConfig:" << exc.displayText() << endl;
        return false;

    } catch (...) {
        cerr << "UnKnow error" << endl;
        return false;
    }

    try {
        GET_CONF_ITEM_CHAR(log4configfile, m_OpenVasConfig.chLog4File, 100);
        GET_CONF_ITEM_INT(moduleid, m_OpenVasConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename, m_OpenVasConfig.chModuleName, 20);
        GET_CONF_ITEM_INT(moduleversion, m_OpenVasConfig.wModuleVersion);
        GET_CONF_ITEM_CHAR(record_separator, m_OpenVasConfig.chRecordSep, 2);
        GET_CONF_ITEM_CHAR(field_separator, m_OpenVasConfig.chFieldSep, 2);
        GET_CONF_ITEM_INT(tasknum_perthread, m_OpenVasConfig.nTaskNumPerThread);
        GET_CONF_ITEM_CHAR(username, m_OpenVasConfig.sUserName, 64);
        GET_CONF_ITEM_CHAR(password, m_OpenVasConfig.sPassWord, 64);
        //GET_CONF_ITEM_CHAR(submodule, m_OpenVasConfig.sSubModule, 128);
        //GET_CONF_ITEM_INT(interval, m_OpenVasConfig.wInterval);

        /*if ( m_OpenVasConfig.wInterval <= 0) {
            m_OpenVasConfig.wInterval = 60;
        }*/

        if (m_OpenVasConfig.nTaskNumPerThread <= 0) {
            m_OpenVasConfig.nTaskNumPerThread = 5;  // 默认每个线程处理5个IP
        }

    } catch (NotFoundException& exc) {
        cerr << "OpenVas LoadConfig:" << exc.displayText() << endl;
        return false;

    } catch (SyntaxException& exc) {
        cerr << "OpenVas LoadConfig:" << exc.displayText() << endl;
        return false;

    } catch (...) {
        cerr << "unKnow error" << endl;
        return false;
    }

    return true;
}

void COpenVasMgr::printConfig() {
    log_info("<=========netservice configure information=========>");
    log_info(" log4configfile=>    [%s]", m_OpenVasConfig.chLog4File);
    log_info(" moduleid=>          [%d]", m_OpenVasConfig.nModuleId);
    log_info(" modulename=>        [%s]", m_OpenVasConfig.chModuleName);
    log_info(" moduleversion=>     [%d]", m_OpenVasConfig.wModuleVersion);
    log_info(" record_separator=>  [%s]", m_OpenVasConfig.chRecordSep);
    log_info(" field_separator=>   [%s]", m_OpenVasConfig.chFieldSep);
    log_info(" tasknum_perthread=> [%d]", m_OpenVasConfig.nTaskNumPerThread);
    log_info(" username=>        [%s]", m_OpenVasConfig.sUserName);
    log_info(" password=>        [%s]", m_OpenVasConfig.sPassWord);
    log_info("<=========netservice configure information=========>");
}

bool COpenVasMgr::Load(void) {
    return true;
}

//调用方下发数据
bool COpenVasMgr::SetData(const PModIntfDataType pDataType, const void* pData, UInt32 dwLength) {
    string strPolicy = *(string*)pData;
    m_vecPolicy.clear();
    m_listPolicy.clear();
    vector<string> vec_task_unit;
    string strRsep = m_OpenVasConfig.chRecordSep;
    string_split(strPolicy, strRsep, vec_task_unit);

    log_info("<=======OpenVas PlOCY Begin=======>");

    int nNum = vec_task_unit.size();  // 策略数量

    for (int i = 0; i < nNum; i++) {
        log_info(" OpenVas POLOCY =>%s", vec_task_unit[i].c_str());
        tag_OpenVas_policy_t tag_policy;
        vector<string> vec_temp;
        string strFsep = m_OpenVasConfig.chFieldSep;

        string_split(vec_task_unit[i], strFsep, vec_temp, false);

        tag_policy.vec_options = vec_temp;

        tag_policy.m_Finished = false;
        tag_policy.m_Result.clear();
        m_vecPolicy.push_back(tag_policy);
        m_listPolicy.push_back(tag_policy);
    }

    log_info("<=======OpenVas PlOCY end=======>");

    if (m_OpenVasConfig.nTaskNumPerThread <= 0) {
        m_OpenVasConfig.nTaskNumPerThread = 1;
    }

    m_pthreadnum = 1;  //当前只使用单线程
    //m_pthreadnum = (int)ceil((double)m_vecPolicy.size() / m_OpenVasConfig.nTaskNumPerThread);

    m_vecThreadPool.reserve(m_pthreadnum);   // 提前分配线程池空间

    return true;
}
//调用方获取数据
void* COpenVasMgr::GetData(const PModIntfDataType pDataType, UInt32& dwRetLen) {
    return NULL;
}

//调用方释放获取到的内存
void COpenVasMgr::FreeData(void* pData) {
    return;
}

//set callback for report function
void COpenVasMgr::SetReportData(pFunc_ReportData pCbReport) {
    m_pFuncReportData = pCbReport;
}
//set call back for fetch function
void COpenVasMgr::SetFetchData(pFunc_FetchData pCbFetch) {
    m_pFuncFetchData = pCbFetch;
}

//Report Data
bool COpenVasMgr::ReportData(const PModIntfDataType pDataType, const void* pData, UInt32 dwLength) {
    if (m_pFuncReportData) {
        return m_pFuncReportData(m_OpenVasConfig.nModuleId, pDataType, pData, dwLength);
    }

    return true;
}
//Fetch Data
const char* COpenVasMgr::FetchData(const PModIntfDataType pDataType, UInt32& dwRetLen) {
    return NULL;
}

//启动模块
bool COpenVasMgr::Start(void) {
    FastMutex::ScopedLock lock(m_ModMutex);

    log_info("openvas start!");

    if (! m_bIsRunning) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);  //初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);     //设置线程属性

        //初始化openvas
        try {
            _pOpenvasImpl = COpenVasImpl::instance();
            _pOpenvasImpl->setName(m_OpenVasConfig.sUserName);
            _pOpenvasImpl->setPasswd(m_OpenVasConfig.sPassWord);
            _pOpenvasImpl->initialize();

        } catch (...) {
            log_error("openvas start error!");
        }

        //TODO 当前只用一个线程处理策略
        pthread_t t_id;
        int err = pthread_create(&t_id, &pa, OnOpenVasHandle, this);

        if (err != 0) {
            log_error("Create OnOpenVasHandle Thread failed! error[%d]:%s", err, strerror(err));
            return false;
        }

        usleep(10000); // 10毫秒，启动线程需要一些执行时间
        m_pthreadnum_alive++;
        log_info("Create OnOpenVasHandle Thread successed!");

        if (pthread_create(&p_thread_timeout, &pa, OnTimeOut, this) != 0) {
            log_info("create OnTimeOut Thread failed!");
            return false;
        }

        usleep(1000);  // 1毫秒
        log_info("Create OnTimeOut Thread successed!");

        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            log_info("Create OnReportHandle Thread failed!");
            return false;
        }

        usleep(1000); // 1毫秒
        log_info("Create OnReportHandle Thread successed!");
    }

    log_info("COpenVasMgr::Start successed!");
    return true;
}

//停止模块
bool COpenVasMgr::Stop(void) {
    FastMutex::ScopedLock lock(m_ModMutex);

    for (int i = 0; i < m_pthreadnum; i++) {
        m_vecThreadPool[i].b_state = false;
    }

    //停定时器
    m_bTimeOutStatus = false;

    log_info("COpenVasMgr::Stop successed!");

    return true;
}

void* COpenVasMgr::OnFetchHandle(void* arg) {
    return NULL;
}

// 上报任务处理结果
void* COpenVasMgr::OnReportHandle(void* arg) {
    COpenVasMgr* cThis = reinterpret_cast<COpenVasMgr*>(arg);
    log_info("OnReportHandle start!");

    while (true) {

        //TODO 增加多线程
        if (cThis->m_pthreadnum == cThis->m_pthreadnum_alive && !cThis->m_bIsRunning) {
            cThis->m_bIsRunning = true; //此时线程池中所有线程已经建立完毕
        }

        vector<tag_OpenVas_policy_t>::iterator iter_temp = cThis->m_vecPolicy.begin();

        list<tag_OpenVas_policy_t>::iterator itReport = cThis->m_listReport.end();

        //TODO 增加线程锁
        if (cThis->m_listReport.size() > 0) {
            itReport = cThis->m_listReport.begin();
            modintf_datatype_t temp;
            temp.nDataType = MODULE_DATA_PERFORMANCE;
            temp.nMainCategory = MODULE_DATA_PERFORMANCE_OPENVAS;

            try {
                if (itReport->m_Finished) {
                    if (! cThis->ReportData(&temp, (void*) & (itReport->m_Result), itReport->m_Result.length())) {
                        log_debug("Report data:%s failed!", itReport->m_Result.c_str());

                    } else {
                        log_debug("Report data:%s success!", itReport->m_Result.c_str());
                    }
                }

                cThis->m_listReport.pop_front();  // 成功删除列表元素

            } catch (...) {
                //TODO ADD :print error list
                log_error("report error");
                throw;
            }
        }

        if (cThis->m_pthreadnum_alive == 0) {
            break;    //处理线程池全部退出后则退出该线程
        }

        if (cThis->m_bIsRunning) {
            sleep(1);    // 不用一直扫描
        }
    }

    cThis->m_bIsRunning = false;
    cThis->m_vecThreadPool.clear();
    log_info("OnReportHandle Thread exit...");

    return NULL;
}

/*
 * @brief :处理策略
 * @param :arg openvasmgr实例
 * @return:NULL 暂不使用
 */
void* COpenVasMgr::OnOpenVasHandle(void* arg) {
    COpenVasMgr* cThis = reinterpret_cast<COpenVasMgr*>(arg);
    log_info("OnOpenVashandle start!");
    cThis->m_vecThreadPool[0].b_state = true;

    while (cThis->m_vecThreadPool[0].b_state) {
        try {
            if (cThis->m_listPolicy.empty()) {
                //log_warn("policy list is empty, please check!");
                usleep(300);
                continue;
            }

            tag_OpenVas_policy_t now = cThis->m_listPolicy.front();

            if (now.vec_options.size() < 2) {
                log_warn("vec_options size less 2!");
                cThis->m_listPolicy.pop_front();
                usleep(100);
                continue;
            }

            log_info("policy cmd:%s", now.vec_options[0].c_str());
            COpenVasImpl::openvas_cmd Cmd = (COpenVasImpl::openvas_cmd)atoi(now.vec_options[0].c_str());

            switch (Cmd) {
                case COpenVasImpl::openvas_cmd_start_task:
                case COpenVasImpl::openvas_cmd_stop_task:
                case COpenVasImpl::openvas_cmd_resume_task:
                case COpenVasImpl::openvas_cmd_get_task:
                case COpenVasImpl::openvas_cmd_get_tasks:
                case COpenVasImpl::openvas_cmd_delete_task:
                case COpenVasImpl::openvas_cmd_del_report:          // 删除报表
                case COpenVasImpl::openvas_cmd_get_report:
                case COpenVasImpl::openvas_cmd_quick_start: {      //quick start a scan
                    string param(now.vec_options[1]);
                    char *tpResult = cThis->_pOpenvasImpl->cmd(Cmd, param.c_str());

                    if (tpResult != NULL) {
                        now.m_Result = tpResult;
                        now.m_Finished = true;
                        cThis->m_listReport.push_back(now);

                    } else {
                        log_warn("invalid result.");
                    }
                }
                break;

                default:
                    log_warn("invalid cmd:%s", now.vec_options[0].c_str());
                    break;
            }

            cThis->m_listPolicy.pop_front();

        } catch (SyntaxException& exc) {
            log_info("syntax exeception, %s", exc.displayText().c_str());
            cThis->m_listPolicy.pop_front();
            continue;

        } catch (NotFoundException& exc) {
            log_info("NotFoundException error, %s", exc.displayText().c_str());
            cThis->m_listPolicy.pop_front();
            continue;

        } catch (Exception& e) {
            log_info("exception error, %s", e.displayText().c_str());
            cThis->m_listPolicy.pop_front();
            continue;

        } catch (...) {

            log_info("unkown error, OnOpenVasHandle exit!");
            cThis->m_listPolicy.pop_front();
            continue;
        }
    }

    cThis->m_pthreadnum_alive --;

    return NULL;
}

//Base64数据加密
string COpenVasMgr::toBase64(string source) {
    stringstream in;
    Base64Encoder b64in(in);
    b64in << source.c_str();
    b64in.close();
    return in.str();
}

void * COpenVasMgr::OnTimeOut(void* arg) {
    COpenVasMgr* cThis = reinterpret_cast<COpenVasMgr*>(arg);

    log_info("OnTimeOut start!");
    int count = 0;
    cThis->m_bTimeOutStatus = true;

    while (cThis->m_bTimeOutStatus) {
        if (!cThis->m_bTimeOut && cThis->_pOpenvasImpl != NULL && cThis->_pOpenvasImpl) {
            usleep(1000000); //1s
            cThis->_pOpenvasImpl->timeClock(300); //6分钟
        }

        if (count > 10000) {
            count = 0;

        } else {
            if (!(count % 5)) {
                char *pData = cThis->_pOpenvasImpl->cmd(COpenVasImpl::openvas_cmd_get_tasks, NULL);
                string data;
                data.clear();

                if (pData == NULL) {
                    continue;
                }

                data = pData;
                modintf_datatype_t temp;
                temp.nDataType = MODULE_DATA_PERFORMANCE;
                temp.nMainCategory = MODULE_DATA_PERFORMANCE_OPENVAS;

                if (! cThis->ReportData(&temp, (void*) & (data), data.length())) {
                    log_debug("Timeout Report data:%s failed!", data.c_str());

                } else {
                    //log_debug("Timeout Report data:%s success!", data.c_str());
                }

            }
        }

        count ++;
    }

    log_info("timeout exist!");
    return NULL;
}

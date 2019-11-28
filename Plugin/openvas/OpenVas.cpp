#include "OpenVas.h"
#include "gsad_base.h"
#include "tracef.h"
#include <string.h>
#include <openvas/misc/openvas_logging.h>
#include <openvas/base/openvas_file.h>
#include <openvas/base/pidfile.h>


//for openvas log
string log(const char* format, ...) {
    char buf[16384];
    memset(buf, 0, sizeof(buf));
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, 16384, format, ap);
    va_end(ap);
    string str = buf;
    return str;
}

int verbose = 0;
//会话超时时间
int session_timeout = 15;

int COpenVasImpl::initialize() {

    init_openvs_func_map();

    if (authenticate(_name.c_str(), _passwd.c_str()) != 0) {
        return -1;
    }

    return 0;
}

void COpenVasImpl::setName(const char* name) {
    _name = name;
}

void COpenVasImpl::setPasswd(const char* passwd) {
    _passwd = passwd;
}

COpenVasImpl* COpenVasImpl::instance() {
    //TODO 需要增加多线程锁
    if (_instance == NULL) {
        _instance = new COpenVasImpl;
    }

    return _instance;
}

int COpenVasImpl::authenticate(const char* username, const char* password) {
    gchar* role, *timezone, *serverity, *capabilities, *language, *pw_warning, *autorefresh;
    GTree* chart_prefs;

    if (_isAuth) {
        log_info("have authenticated!");
        return -1;
    }

    // 返回0表示成功
    int ret = authenticate_omp(username,
                               password,
                               &role,
                               &timezone,
                               &serverity,
                               &capabilities,
                               &language,
                               &pw_warning,
                               &chart_prefs,
                               &autorefresh);

    if (ret != 0) {
        log_info("authenticate failed! username:%s, password:%s", username, password);
        return -1;

    } else {
        log_info("Authenticate success! username:%s, password:%s", username, password);
    }

    user_t* user = user_add(username, password, timezone, serverity,
                            role, capabilities, language, pw_warning,
                            chart_prefs, autorefresh, "127.0.0.1");
    _credentials = (credentials_t*)calloc(1, sizeof(credentials_t));
    _credentials = credentials_new(user, language, user->address);

    return 0;
}

COpenVasImpl* COpenVasImpl::_instance = NULL;

/*
 * @brief:quick start 创建扫描任务
 * @param host 目标主机
 */
char* COpenVasImpl::quick_start(const char* host) {
    params_t* params = params_new();
    params_add(params, "cmd", "wizard");
    params_add(params, "name", "quick_first_scan");

    param_t* target_param = (param_t*)g_malloc0(sizeof(param_t));
    target_param->values = params_new();
    params_add(target_param->values, "hosts", host);
    target_param->valid = 0;
    target_param->valid_utf8 = 0;
    g_hash_table_insert(params, g_strdup("event_data:"), target_param);
    params_add(params, "next", "get_tasks");

    _credentials->params = params;
    char* ret = run_wizard_omp(_credentials, params);

    params_free(params);
    return ret;
}

char* COpenVasImpl::start_task(const char* task_id) {
    params_t* params = params_new();
    params_add(params, "cmd", "start_task");
    params_add(params, "task_id", task_id);
    params_add(params, "next", "get_task");
    _credentials->params = params;

    char* ret = start_task_omp(_credentials, params);

    if (ret == NULL) {
        printf("start task [%s] failed!", task_id);
    }

    log_debug("start_task result:%s", ret);
    params_free(params);
    return ret;
}

/**
 * @brief delete操作后会执行get_tasks步骤
 */
char* COpenVasImpl::delete_task(const char* task_id) {
    params_t* params = params_new();
    params_add(params, "cmd", "delete_task");
    params_add(params, "task_id", task_id);
    _credentials->params = params;
    char* ret = delete_task_omp(_credentials, params);
    params_free(params);
    return ret;
}

char* COpenVasImpl::get_tasks(const char* value) {
    log_info("get tasks");
    params_t* params = params_new();
    params_add(params, "cmd", "get_tasks");
    //一次传送200条
    params_add(params, "filter", "rows=200");
    _credentials->params = params;
    char* ret = get_tasks_omp(_credentials, params);
    params_free(params);
    return ret;
}

char* COpenVasImpl::get_task(const char* task_id) {
    params_t* params = params_new();
    params_add(params, "cmd", "get_task");
    params_add(params, "task_id", task_id);
    _credentials->params = params;
    char* ret = get_task_omp(_credentials, params);

    params_free(params);
    return ret;
}

/*
 * @brief 停止任务，omp6.0取消了pause_task
 */
char* COpenVasImpl::stop_task(const char* task_id) {
    params_t* params = params_new();
    params_add(params, "cmd", "stop_task");
    params_add(params, "task_id", task_id);
    params_add(params, "next", "get_tasks");
    _credentials->params = params;
    char* ret = stop_task_omp(_credentials, params);
    params_free(params);
    return ret;
}

char* COpenVasImpl::resume_task(const char* task_id) {
    params_t* params = params_new();
    params_add(params, "cmd", "resume_task");
    params_add(params, "task_id", task_id);
    params_add(params, "next", "get_tasks");
    _credentials->params = params;
    char* ret = resume_task_omp(_credentials, params);
    params_free(params);
    return ret;
}

char* COpenVasImpl::get_report(const char* report_id) {

    params_t* params = params_new();
    params_add(params, "cmd", "get_report");
    params_add(params, "report_id", report_id);

    //params_add(params, "report_format_id", "a3810a62-1f62-11e1-9219-406186ea4fc5");
    params_add(params, "report_format_id", XML);

    params_add(params, "levels", "hmlgf");
    params_add(params, "result_hosts_only", "1");
    _credentials->params = params;

    size_t len = 0;
    char* ret = get_report_omp(_credentials, params, &len, NULL, NULL);

    if (ret == NULL) {
        printf("get report [%s] failed!", report_id);
    }

    params_free(params);
    return ret;
}
user_t*
COpenVasImpl::user_add(const gchar* username, const gchar* password, const gchar* timezone,
                       const gchar* severity, const gchar* role, const gchar* capabilities,
                       const gchar* language, const gchar* pw_warning, GTree* chart_prefs,
                       const gchar* autorefresh, const char* address) {
    user_t* user = NULL;
    unsigned int index;
    //g_mutex_lock(mutex);

    for (index = 0; index < _users->len; index++) {
        user_t* item;
        item = (user_t*) g_ptr_array_index(_users, index);

        if (strcmp(item->username, username) == 0) {
            if (time(NULL) - item->time > (session_timeout * 60)) {
                g_ptr_array_remove(_users, (gpointer) item);
            }
        }
    }

    user = (user_t*)g_malloc(sizeof(user_t));
    user->cookie = openvas_uuid_make();
    user->token = openvas_uuid_make();
    user->username = g_strdup(username);
    user->password = g_strdup(password);
    user->role = g_strdup(role);
    user->timezone = g_strdup(timezone);
    user->severity = g_strdup(severity);
    user->capabilities = g_strdup(capabilities);
    user->language = language ? g_strdup(language) : NULL;
    user->pw_warning = pw_warning ? g_strdup(pw_warning) : NULL;
    user->chart_prefs = chart_prefs;
    user->autorefresh = g_strdup(autorefresh);
    user->last_filt_ids = g_tree_new_full((GCompareDataFunc) g_strcmp0,
                                          NULL, g_free, g_free);
    g_ptr_array_add(_users, (gpointer) user);
    set_language_code(&user->language, language);
    user->time = time(NULL);
    user->charts = 0;

    /*
    if (guest_username) {
        user->guest = strcmp(username, guest_username) ? 0 : 1;

    } else {
        user->guest = 0;
    }*/
    //默认用admin
    user->guest = 0;

    user->address = g_strdup(address);
    return user;
}

credentials_t*
COpenVasImpl::credentials_new(user_t* user, const char* language, const char* client_address) {
    credentials_t* credentials;

    assert(user->username);
    assert(user->password);
    assert(user->role);
    assert(user->timezone);
    assert(user->capabilities);
    assert(user->token);
    credentials = (credentials_t*)g_malloc0(sizeof(credentials_t));
    credentials->username = g_strdup(user->username);
    credentials->password = g_strdup(user->password);
    credentials->role = g_strdup(user->role);
    credentials->timezone = g_strdup(user->timezone);
    credentials->severity = g_strdup(user->severity);
    credentials->capabilities = g_strdup(user->capabilities);
    credentials->token = g_strdup(user->token);
    credentials->charts = user->charts;
    credentials->chart_prefs = user->chart_prefs;
    credentials->pw_warning = user->pw_warning ? g_strdup(user->pw_warning)
                              : NULL;
    credentials->language = g_strdup(language);
    credentials->autorefresh = user->autorefresh
                               ? g_strdup(user->autorefresh) : NULL;
    credentials->last_filt_ids = user->last_filt_ids;
    credentials->client_address = g_strdup(client_address);

    return credentials;
}


void COpenVasImpl::init_openvs_func_map() {
    if (_openvas_task_func_map.size() == 0) {
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_start_task, &COpenVasImpl::start_task));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_stop_task, &COpenVasImpl::stop_task));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_resume_task, &COpenVasImpl::resume_task));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_get_task, &COpenVasImpl::get_task));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_get_tasks, &COpenVasImpl::get_tasks));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_delete_task, &COpenVasImpl::delete_task));
        //_openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_del_report, &COpenVasImpl::del_report));
        //_openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_delete_task, &COpenVasImpl::delete_task));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_quick_start, &COpenVasImpl::quick_start));
        _openvas_task_func_map.insert(make_pair(COpenVasImpl::openvas_cmd_get_report, &COpenVasImpl::get_report));
    }

}

char* COpenVasImpl::cmd(openvas_cmd c, const char* param) {
    std::map<openvas_cmd, _openvas_cmd_func>::iterator it = _openvas_task_func_map.find(c);

    log_info("openvas_cmd:%d, map size:%d", c, _openvas_task_func_map.size());

    if (it != _openvas_task_func_map.end()) {
        COpenVasImpl::_openvas_cmd_func pFunc = it->second;
        _timeClock = 0;
        //return (this->*pFunc)(param);
        return (instance()->*pFunc)(param);

    } else {
        log_info("invalid cmd:can't match cmd.");
        return NULL;
    }
}

bool COpenVasImpl::timeClock(unsigned int timeout) {
    _timeClock ++;

    if (timeout > _timeClock) {
        return false;
    }

    //超时则认证
    if (authenticate(_name.c_str(), _passwd.c_str()) != 0) {
        return -1;
    }

    return true;
}


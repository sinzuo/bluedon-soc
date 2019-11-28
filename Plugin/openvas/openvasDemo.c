#include "gsad_base.h"
#include "gsad_omp.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "openvasDemo.h"
#include <openvas/misc/openvas_logging.h>
#include <openvas/base/openvas_file.h>
#include <openvas/base/pidfile.h>
#include <openvas/misc/openvas_uuid.h>
#include "xslt_i18n.h"

/**
 * @brief report format
 */
#define TXT "a3810a62-1f62-11e1-9219-406186ea4fc5"
#define PDF "c402cc3e-b531-11e1-9163-406186ea4fc5"
#define XML "5057e5cc-b825-11e4-9d0e-28d24461215b"
#define Anonymous_XML "a994b278-1f62-11e1-96ac-406186ea4fc5"

static GMutex *mutex = NULL;
GPtrArray *users = NULL;
/**
 * @brief Max number of minutes between activity in a session.
 */
#define SESSION_TIMEOUT 15

int session_timeout = 15;
char * guest_username = "test";
int verbose = 0;
/**
 * @brief User information structure, for sessions.
 */
struct user {
    char *cookie;        ///< Cookie token.
    char *token;         ///< Request session token.
    gchar *username;     ///< Login name.
    gchar *password;     ///< Password.
    gchar *role;         ///< Role.
    gchar *timezone;     ///< Timezone.
    gchar *severity;     ///< Severity class.
    gchar *capabilities; ///< Capabilities.
    gchar *language;     ///< User Interface Language, in short form like "en".
    gchar *pw_warning;   ///< Password policy warning.
    char *address;       ///< Client's IP address.
    time_t time;         ///< Login time.
    int charts;          ///< Whether to show charts for this user.
    GTree *chart_prefs;  ///< Chart preferences.
    gchar *autorefresh; ///< Auto-Refresh interval
    GTree *last_filt_ids;///< Last used filter ids.
    int guest;           ///< Whether the user is a guest.
};

/**
 * @brief User information type, for sessions.
 */
typedef struct user user_t;

static credentials_t *credentials = NULL;

char * Initialise(const char* guest, const char* passwd) {

}


credentials_t * credentials_new(user_t *user, const char *language, const char *client_address);
user_t* user_add(const gchar *username, const gchar *password, const gchar *timezone,
                 const gchar *severity, const gchar *role, const gchar *capabilities,
                 const gchar *language, const gchar *pw_warning, GTree *chart_prefs,
                 const gchar *autorefresh, const char *address);

int main(int argc, char *argv[]) {
    const gchar * cAddr = "127.0.0.1";
    //credentials_t *credentials = NULL;
    int cPort = 9392;

    const gchar *guest_username = strdup("admin");
    const gchar *guest_password = strdup("123456");
    gchar *role, *timezone, *serverity, *capabilities, *language, *pw_warning, *autorefresh;
    GTree *chart_prefs;
    // TODO for init
    users = g_ptr_array_new();
    mutex = g_mutex_new();
    int i = authenticate_omp(guest_username,
                             guest_password,
                             &role,
                             &timezone,
                             &serverity,
                             &capabilities,
                             &language,
                             &pw_warning,
                             &chart_prefs,
                             &autorefresh);

    user_t * user = user_add("admin", "123456", timezone, serverity,
                             role, capabilities, language, pw_warning,
                             chart_prefs, autorefresh, "127.0.0.1");
    credentials = (credentials_t *)calloc(1, sizeof(credentials_t));
    credentials = credentials_new(user, language, user->address);

    // create scanner
    params_t *nscan_params = params_new();
    params_add(nscan_params, "name", "test1");
    params_add(nscan_params, "comment", "just for test");
    params_add(nscan_params, "host", "localhost");
    params_add(nscan_params, "port", "9392");
    params_add(nscan_params, "scanner_type", "OpenVAS Scanner");
    params_add(nscan_params, "ca_pub", "");
    params_add(nscan_params, "key_pub", "");
    params_add(nscan_params, "key_priv", "");

    params_t *task_params = params_new();
    //params_add(task_params, "tag_name", "0");
    //params_add(task_params, "tag_value", "0");

    g_warning("create_task_omp start =======================");
    //char *ret = create_task_omp(credentials, task_params);


    /*
     * TODO TEST
     */
    char * ret = NULL;
    //TODO 开始任务
    //ret = start_task("3bfe7c0e-3ba0-4518-bb03-4b6a07deed00");
    //TODO ADD CODE 获取任务
    //ret = get_tasks();
    //TODO ADD CODE 暂停任务
    //ret = stop_task("3bfe7c0e-3ba0-4518-bb03-4b6a07deed00");
    //TODO resume stopped task
    //ret = resume_task("3bfe7c0e-3ba0-4518-bb03-4b6a07deed00");
    //TODO get task
    //ret = get_task("3bfe7c0e-3ba0-4518-bb03-4b6a07deed00");
    //ret = get_report("d58612c1-7ba7-4dc5-a61e-d82c3fd41536");
    //ret = del_task("aad01157-0057-4f2d-aa70-6b7b62aa12bd");
    //ret = create_task("");

    quick_start("172.16.12.188");

    params_free(task_params);

    printf("ret:%s\n---------------end2", ret);

    return 0;
}

char* get_report(const char* report_id){

    params_t * params = params_new();
    params_add(params, "cmd", "get_report");
    params_add(params, "report_id", report_id);

    params_add(params, "report_format_id", "a3810a62-1f62-11e1-9219-406186ea4fc5");

    params_add(params, "levels", "hmlgf");
    params_add(params, "result_hosts_only", "1");
    credentials->params = params;

    size_t len = 0;
    char* ret = get_report_omp(credentials, params, &len, NULL, NULL);

    if (ret == NULL) {
        printf("get report [%s] failed!", report_id);
    }

    params_free(params);
    return ret;
}


/**
 * @brief create a task
 */
/*char * create_task(const char* taskid ) {
    params_t *params = params_new();

    params_add(task_params, "name", "task1");
    params_add(task_params, "comment", "-------");
    params_add(task_params, "target_id", "fb66a8e1-fbdb-489c-b8c0-9251f6f6de1a");
    params_add(task_params, "scanner_type", "2"); // TODO
    params_add(task_params, "hosts_ordering", "");
    params_add(task_params, "slave_id_optional", "");
    params_add(task_params, "schedule_id_optional", "");
    params_add(task_params, "schedule_periods", "0");
    params_add(task_params, "scanner_id", "08b69003-5fc2-4037-a479-93b440211c73");
    params_add(task_params, "config_id", "daba56c8-73ec-11df-a475-002264764cea");
    params_add(task_params, "in_assets", "yes");
    params_add(task_params, "max_checks", "10");
    params_add(task_params, "source_iface", "");
    params_add(task_params, "auto_delete", "0");
    params_add(task_params, "auto_delete_data", "0");
    params_add(task_params, "max_hosts", "30");
    params_add(task_params, "alterable", "0");
    params_add(task_params, "submit_plus", "+");
}*/

char * quick_start(const char *host) {
    params_t *params = params_new();
    params_add(params, "cmd", "wizard");
    params_add(params, "name", "quick_first_scan");

    param_t *target_param = g_malloc0(sizeof(param_t));
    target_param->values = params_new();
    params_add(target_param->values, "hosts", host);
    target_param->valid = 0;
    target_param->valid_utf8 = 0;
    g_hash_table_insert(params, g_strdup("event_data:"), target_param);

    credentials->params = params;
    char *ret = run_wizard_omp(credentials, params);
    return ret;
}
/*
 * @brief start a openvas task by taskid.
 */

char* start_task(const char * taskid) {

    params_t * params = params_new();
    params_add(params, "cmd", "start_task");
    params_add(params, "task_id", taskid);
    params_add(params, "next", "get_task");
    credentials->params = params;

    char* ret = start_task_omp(credentials, params);

    if (ret == NULL) {
        printf("start task [%s] failed!", taskid);
    }

    params_free(params);
    return ret;
}

char * get_tasks() {
    params_t* params = params_new();
    params_add(params, "cmd", "get_tasks");
    credentials->params = params;
    char *ret = get_tasks_omp(credentials, params);
    params_free(params);
    return ret;
}

char *del_task(const char* taskid) {
    params_t* params = params_new();
    params_add(params, "cmd", "delete_task");
    params_add(params, "task_id", taskid);
    credentials->params = params;
    char *ret = delete_task_omp(credentials, params);
    params_free(params);
    return ret;
}

/*
 * @brief 停止任务，omp6.0取消了pause_task
 */
char *stop_task(const char* taskid) {
    params_t* params = params_new();
    params_add(params, "cmd", "stop_task");
    params_add(params, "task_id", taskid);
    params_add(params, "next", "get_tasks");
    credentials->params = params;
    char *ret = stop_task_omp(credentials, params);
    params_free(params);
    return ret;
}

char * resume_task(const char* taskid){
    params_t *params = params_new();
    params_add(params, "cmd", "resume_task");
    params_add(params, "task_id", taskid);
    params_add(params, "next", "get_tasks");
    //params_add(params, "task_id", taskid);
    credentials->params = params;
    char *ret = resume_task_omp(credentials, params);
    params_free(params);
    return ret;
}

char * get_task(const char* taskid) {
    params_t *params = params_new();
    params_add(params, "cmd", "get_task");
    params_add(params, "task_id", taskid);
    credentials->params = params;
    char *ret = get_task_omp(credentials, params);

    params_free(params);
    return ret;
}

credentials_t *
credentials_new(user_t *user, const char *language, const char *client_address) {
    credentials_t *credentials;

    assert(user->username);
    assert(user->password);
    assert(user->role);
    assert(user->timezone);
    assert(user->capabilities);
    assert(user->token);
    credentials = g_malloc0(sizeof(credentials_t));
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


user_t *
user_add(const gchar *username, const gchar *password, const gchar *timezone,
         const gchar *severity, const gchar *role, const gchar *capabilities,
         const gchar *language, const gchar *pw_warning, GTree *chart_prefs,
         const gchar *autorefresh, const char *address) {
    user_t *user = NULL;
    int index;
    g_mutex_lock(mutex);

    for (index = 0; index < users->len; index++) {
        user_t *item;
        item = (user_t*) g_ptr_array_index(users, index);

        if (strcmp(item->username, username) == 0) {
            if (time(NULL) - item->time > (session_timeout * 60)) {
                g_ptr_array_remove(users, (gpointer) item);
            }
        }
    }

    user = g_malloc(sizeof(user_t));
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
    g_ptr_array_add(users, (gpointer) user);
    set_language_code(&user->language, language);
    user->time = time(NULL);
    user->charts = 0;

    if (guest_username) {
        user->guest = strcmp(username, guest_username) ? 0 : 1;

    } else {
        user->guest = 0;
    }

    user->address = g_strdup(address);
    return user;
}


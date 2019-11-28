#ifndef _OPENVAS_H_
#define _OPENVAS_H_

#include "gsad_base.h"
#include "gsad_omp.h"
#include <string>
#include "config/BDOptions.h"
#include <uuid/uuid.h>

// 不加extern无法调用
#ifdef __cplusplus
extern "C" {
#endif

//#include <openvas/misc/openvas_uuid.h>

#ifdef __cplusplus
}
#endif
using namespace std;

#define  OPENVAS_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("OPENVAS"), str);
#define  OPENVAS_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("OPENVAS"), str);
#define  OPENVAS_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("OPENVAS"), str);
#define  OPENVAS_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("OPENVAS"), str);

#define  OPENVAS_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("OPENVAS"), #str);
#define  OPENVAS_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("OPENVAS"), #str);
#define  OPENVAS_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("OPENVAS"), #str);
#define  OPENVAS_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("OPENVAS"), #str);

string log(const char* format, ...);

#define log_info(format, args...) LOG4CXX_INFO(log4cxx::Logger::getLogger("OPENVAS"), log(format, ##args));
#define log_debug(format, args...) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("OPENVAS"), log(format, ##args));
#define log_warn(format, args...) LOG4CXX_WARN(log4cxx::Logger::getLogger("OPENVAS"), log(format, ##args));
#define log_error(format, args...) LOG4CXX_ERROR(log4cxx::Logger::getLogger("OPENVAS"), log(format, ##args));

/**
 * @brief report format
 */
#define TXT "a3810a62-1f62-11e1-9219-406186ea4fc5"
#define PDF "c402cc3e-b531-11e1-9163-406186ea4fc5"
#define XML "5057e5cc-b825-11e4-9d0e-28d24461215b"
#define Anonymous_XML "a994b278-1f62-11e1-96ac-406186ea4fc5"

/**
 * @brief user struct
 */
typedef struct user {
    char* cookie;        ///< Cookie token.
    char* token;         ///< Request session token.
    gchar* username;     ///< Login name.
    gchar* password;     ///< Password.
    gchar* role;         ///< Role.
    gchar* timezone;     ///< Timezone.
    gchar* severity;     ///< Severity class.
    gchar* capabilities;  ///< Capabilities.
    gchar* language;     ///< User Interface Language, in short form like "en".
    gchar* pw_warning;   ///< Password policy warning.
    char* address;       ///< Client's IP address.
    time_t time;         ///< Login time.
    int charts;          ///< Whether to show charts for this user.
    GTree* chart_prefs;  ///< Chart preferences.
    gchar* autorefresh;  ///< Auto-Refresh interval
    GTree* last_filt_ids;  ///< Last used filter ids.
    int guest;           ///< Whether the user is a guest.
} user_t;

/**
 * @brief User information type, for sessions.
 */
typedef struct user user_t;
class COpenVasImpl {
public:
    typedef char* (COpenVasImpl::*_openvas_cmd_func)(const char*);

    typedef enum __Cmd {
        openvas_cmd_none        = 0x00,
        openvas_cmd_start_task  = 0x01,
        openvas_cmd_stop_task   = 0x02,
        openvas_cmd_resume_task = 0x03,
        openvas_cmd_get_task    = 0x04,
        openvas_cmd_get_tasks   = 0x05,
        openvas_cmd_delete_task = 0x06,
        openvas_cmd_get_report  = 0x07,
        openvas_cmd_del_report  = 0x08,
        openvas_cmd_quick_start = 0x09
    } openvas_cmd;
public:
    COpenVasImpl() {
        _isAuth = false;
        _users = g_ptr_array_new();
        _name = "admin";
        _passwd = "123456";
        _timeClock = 0;
    }
    COpenVasImpl(const char* username, const char* passwd) {
        _isAuth = false;
        _users = g_ptr_array_new();
        _name = username;
        _passwd = passwd;
    }
    ~COpenVasImpl() {}

public:
    static COpenVasImpl* instance();
    int initialize();
    void init_openvs_func_map();
    char* cmd(openvas_cmd c, const char* param);


    char* quick_start(const char* host);
    char* start_task(const char* task_id);
    char* delete_task(const char* task_id);
    char* get_tasks(const char* value);
    char* get_task(const char* task_id);
    char* stop_task(const char* task_id);
    char* resume_task(const char* task_id);
    char* get_report(const char* report_id);

    void setName(const char* name);
    void setPasswd(const char* passwd);
    /**
     * @brief 判断是否超时，用于超时认证
     * @timeout <= 0 返回false 否则返回true 单件s
     */
    bool timeClock(unsigned int timeout);
private:

    /**
     * @brief 创建用户
     */
    user_t*
    user_add(const gchar* username, const gchar* password, const gchar* timezone,
             const gchar* severity, const gchar* role, const gchar* capabilities,
             const gchar* language, const gchar* pw_warning, GTree* chart_prefs,
             const gchar* autorefresh, const char* address);
    /**
     * @brief 创建credentials
     */
    credentials_t*
    credentials_new(user_t* user, const char* language, const char* client_address);

    /**
     * @brief authenticate by username & passwd
     * @param username
     * @param password
     */
    int authenticate(const char* username, const char* passwd);

private:
    bool _isAuth;                           // 是否已经认证过
    credentials_t* _credentials;            // 认证信息
    static COpenVasImpl* _instance;
    GPtrArray* _users;                      // 存储用户信息
    string _name;                           // 用户名
    string _passwd;                         // 密码
    unsigned int _timeClock;                  // 超时计数器
    std::map<openvas_cmd, _openvas_cmd_func> _openvas_task_func_map;

};

#endif

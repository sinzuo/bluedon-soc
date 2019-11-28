/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_servicestatus.cpp
 * @brief   soc所在的主机及相关后台服务的控制操作, snmp,syslog-ng,tomcat,日志采集器及soc
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "bd_servicestatus.h"
#ifdef WIN32
#include <windows.h>
#else
//#include <unistd.h>
#include <dirent.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <list>
#include <string>
using namespace std;
#include "ace/OS.h"
#include "ace/Process_Manager.h"
//#include "utils/bd_common.h"
//#include "bd_log_macros.h"
#include "bd_action_local.h"
#include "bd_jni.h"


int get_privilege() {
#ifdef WIN32
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // 获取当前应用程序进程句柄
    // Get a token for this process. 打开当前进程的访问令牌句柄(OpenProcessToken函数调用失败返回值为零)
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return -1;
    }

    // Get the LUID for the shutdown privilege.
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1; // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; // 权限的属性,SE_PRIVILEGE_ENABLED为使能该权限

    // Get the shutdown privilege for this process.
    // 调整访问令牌里的指定权限(AdjustTokenPrivileges函数调用失败返回值为零)
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS) { // 查看权限是否调整成功
        return -1;
    }

    return 0;
#else
    return 0;
#endif
}

int bd_set_time(const string& setTime) {
    //BD_TRACE( bd_servicestatus, bd_set_time );

    if (0 != get_privilege()) {
        log_error("get privilege failed");
        return -1;
    }

#ifdef WIN32
    SYSTEMTIME systemTime;
    GetLocalTime(&systemTime);
    sscanf(setTime.c_str(), "%4d-%2d-%2d %2d:%2d:%2d", &systemTime.wYear, &systemTime.wMonth, &systemTime.wDay,
           &systemTime.wHour, &systemTime.wMinute, &systemTime.wSecond);

    if (!SetLocalTime(&systemTime)) {
        log_error("SetLocalTime failed, setDate : %s", setTime);
        return -1;
    }

    return 0;
#else
    std::string command("date -s \"");
    command += setTime;
    command += "\"";
    bd_action_local local(command.c_str(), "");
    int ret = local.execute();
    bd_action_local timesave("clock -w", "");
    timesave.execute();
    log_info("%s End, ret is : %d, %s", command.c_str(), ret, 0 == ret ? "success" : "fail");
    return ret;
#endif
}

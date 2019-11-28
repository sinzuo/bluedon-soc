/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_jni_init.cpp
 * @brief   调用jni之前,初始化日志及数据库使用环境
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "bd_jni_init.h"
#include <string>
using namespace std;
#include "ace/Guard_T.h"
//#include "config/BDOptions.h"

bd_jni_init bd_jni_init::s_instance;
//ACE_Thread_Mutex bd_jni_init::s_mutex;
ACE_Thread_Mutex bd_jni_init::s_mutex_init;

#define BD_SOCSERVER_JNI_LOG4_FILE "/usr/local/socserver/conf/jniloginfo.properties"

bd_jni_init::bd_jni_init() : m_isInit(false) {
}

bd_jni_init::~bd_jni_init() {
}

bd_jni_init* bd_jni_init::instance() {
    return &s_instance;
}

int bd_jni_init::init(const string& basePath) {
    if (basePath.empty()) {
        return -1;
    }

    int ret = 0;

    if (! m_isInit) {
        ACE_Guard<ACE_Thread_Mutex> guard(s_mutex_init);

        if (! m_isInit) {
            ret = _initEnv(basePath);
            m_isInit = (0 == ret ? true : false);
        }
    }

    return ret;
}

int bd_jni_init::_initEnv(const string& basePath) {

    if (access(BD_SOCSERVER_JNI_LOG4_FILE, F_OK) != 0) {
        return false;

    } else {
        try {
            log4cxx::PropertyConfigurator::configure(BD_SOCSERVER_JNI_LOG4_FILE);

        } catch (Exception&) {
            return false;
        }
    }

    return 0;
}


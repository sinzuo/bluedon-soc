/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_jni_init.h
 * @brief   调用jni之前,初始化日志及数据库使用环境
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef BD_JNI_INIT_H_
#define BD_JNI_INIT_H_

#include <string>
using namespace std;

#include "ace/Thread_Mutex.h"
#include "config/BDOptions.h"

class bd_jni_init {
public:
    static bd_jni_init* instance();

    /*
     *  执行初始化,double check防止多线程多次生成
     *  @param basePath     soc执行程序所在的上一层路径
     *  @return             0: 成功 -1:失败
     */
    int init(const string& basePath);

private:
    /*
     *  初始化日志及数据库使用环境
     *  @param basePath     soc执行程序所在的上一层路径
     *  @return             0: 成功 -1:失败
     */
    int _initEnv(const string& basePath);

    bd_jni_init();
    ~bd_jni_init();

private:
    static bd_jni_init s_instance;
    static ACE_Thread_Mutex s_mutex_init;
    bool m_isInit;
};


#endif // BD_JNI_INIT_H_


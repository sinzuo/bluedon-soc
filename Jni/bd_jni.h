/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_jni.h
 * @brief   各种jni调用
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/


#ifndef BD_JNI_H_
#define BD_JNI_H_

#include <string>
#include <jni.h>
#include "config/BDOptions.h"

std::string log(const char* format, ...);

#define log_info(format, args...) LOG4CXX_INFO(log4cxx::Logger::getLogger("jni"), log(format, ##args));
#define log_debug(format, args...) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("jni"), log(format, ##args));
#define log_warn(format, args...) LOG4CXX_WARN(log4cxx::Logger::getLogger("jni"), log(format, ##args));
#define log_error(format, args...) LOG4CXX_ERROR(log4cxx::Logger::getLogger("jni"), log(format, ##args));

#ifdef __cplusplus
extern "C"
{
#endif

///////////////////
//  soc 控制    //
///////////////////

/**
 * 更改SOC服务器时间
 * @param jdate      需改成的时间: YYYY-MM-DD HH:mm:SS
 * @return           0 : 成功, 其它 : 失败
 */
JNIEXPORT jint JNICALL Java_com_bluedon_core_jni_DevJNI_setServerTime
(JNIEnv* env, jobject obj, jstring jbasepath, jstring jdate);

/*
 * 获取CPU使用率
 * @parma
 * @return   [0,100]:内存使用率; 其它:失败
 */
JNIEXPORT jint JNICALL Java_com_bluedon_core_jni_DevJNI_getCPU
(JNIEnv *env, jobject obj, jstring jbasepath);

/*
 * 获取内存使用率
 * @parma
 * @return   [0,100]:内存使用率; 其它:失败
 */
JNIEXPORT jint JNICALL Java_com_bluedon_core_jni_DevJNI_getMemory
(JNIEnv *env, jobject obj, jstring jbasepath);

/*
 * 获取磁盘使用率
 * @parma
 * @return   挂载点,总容量(KB),已用容量(KB):/,51606140,11765380;/boot,495844,33406;/home,249421716,11604052
 */
JNIEXPORT jstring JNICALL Java_com_bluedon_core_jni_DevJNI_getHarddisk
(JNIEnv *env, jobject obj, jstring jbasepath);

/**
 * 获取SOC服务器 /etc/sysconfig/network-scripts/ifcfg-eth* 配置文件信息
 * @param
 * @return jstring "DEVNAME,IPADDR,NETMASK,GATEWAY,ONBOOT,DNS1,DNS12[;...]"
 */
JNIEXPORT jstring JNICALL Java_com_bluedon_core_jni_DevJNI_readIfcfgFile
(JNIEnv* env, jobject obj, jstring jbasepath);

/**
 * 修改服务器 /etc/sysconfig/network-scripts/ifcfg-eth* 配置文件
 * @param jifcfginfo     需改成的网口信息:"devName,IPAddr,netMask,gateWay,onBoot,dns1,dns2"
 *                        ONBOOT:引导时是否激活[yes|no],如果为空,则删除,但是","标志仍保留
 * @return               0 : 成功, 其它 : 失败
 */
JNIEXPORT jint JNICALL Java_com_bluedon_core_jni_DevJNI_modifyIfcfgFile
(JNIEnv* env, jobject obj, jstring jbasepath, jstring jifcfginfo);


/**
 * 获取IP地址对应地理位置
 * @param jip            传入获取地理位置的IP
 * @return               0 : 成功，其它 : 失败
 */
JNIEXPORT jstring JNICALL Java_com_bluedon_core_jni_DevJNI_GeoIPRegion
(JNIEnv* env, jobject obj, jstring jip);

/**
 * 服务器蜂鸣器警报控制
 * @param jmode          控制模式, 用于控制激活蜂鸣器发出响声, 或停止发声
 *                        0: 停止蜂鸣器   1(或其他值): 蜂鸣器开始发声一段时间
 * @return               0 : 成功, 其它 : 失败
 */
JNIEXPORT jint JNICALL Java_com_bluedon_core_jni_DevJNI_buzzerControl
(JNIEnv* env, jobject obj, jint jmode);

#ifdef __cplusplus
}
#endif

#endif // BD_JNI_H_


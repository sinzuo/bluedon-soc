/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 * 
 * @file    bd_jni.cpp
 * @brief   各种jni调用
 * 
 *
 * @version 1.0 
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "bd_jni.h"
#include <string>
#include "bd_action_local.h"
using namespace std;

#include "ace/OS.h"

#include "bd_jni_updatenelogininfo.h"

#include "bd_jni_init.h"
#include "bd_jni_network.h"
#include "bd_sysstat.h"
#include "bd_servicestatus.h"
#include "bd_buzzer.h"
#include "utils/bd_common.h"
#include "GeoIP.h"
#include "GeoIPCity.h"


string log(const char* format, ...) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, 4096, format, ap);
    va_end(ap);
    string str = buf;
    return str;
}

#define BD_SOCSERVER_BASEPATH "/usr/local/socserver"

//#define DATAFILE "/root/SOC/soc5.0/lib_third/GeoIP/data/GeoLiteCity.dat"
//南基
//#define DATAFILE "/root/SOC/soc5.0/lib_third/GeoIP/data/GeoLiteCity.dat"
//soc5.0.1
#define DATAFILE "/root/SOC/soc5.1/lib_third/GeoIP/data/GeoLiteCity.dat"

///////////////////
//  soc 控制    //
///////////////////

jint JNICALL Java_com_bluedon_core_jni_DevJNI_setServerTime
( JNIEnv* env, jobject obj, jstring jbasepath, jstring jdate )
{
    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )
        return -1;
 
    const char* date = env->GetStringUTFChars( jdate, 0 );
    if ( 0 == date )
    {
        log_error( "GetStringUTFChars return NULL" );
        return -1;
    }

    log_debug("setDate: [%s]", date);
    int ret = bd_set_time( date );
    env->ReleaseStringUTFChars( jdate, date );
    return ret;
}

jint JNICALL Java_com_bluedon_core_jni_DevJNI_getCPU
( JNIEnv *env, jobject obj, jstring jbasepath )
{
    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )
        return -1;

    return bd_get_cpustat();
}

jint JNICALL Java_com_bluedon_core_jni_DevJNI_getMemory
( JNIEnv *env, jobject obj, jstring jbasepath )
{
    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )
        return -1;

    return bd_get_memstat();
}

jstring JNICALL Java_com_bluedon_core_jni_DevJNI_getHarddisk
( JNIEnv *env, jobject obj, jstring jbasepath )
{
    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )
        return 0;

    string diskstatus;
    if ( 0 != bd_get_diskstat(&diskstatus) )
    {
        log_error( "get cpu status fail" );
        return 0;
    } 
    log_debug("hard disk status : [%s]", diskstatus.c_str());
    jstring jdiskstatus = env->NewStringUTF( diskstatus.c_str() );
    if ( NULL == jdiskstatus ) 
    {
        if (env->ExceptionCheck()) {   
            log_error( "NewStringUTF fail" );
        }
    }
    return jdiskstatus;
}

jstring JNICALL Java_com_bluedon_core_jni_DevJNI_readIfcfgFile
(JNIEnv *env, jobject obj, jstring jbasepath)
{

    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )   
        return 0;

    std::string ifcfginfo;
    int ret = bd_network_readIfcfg( &ifcfginfo );
    if ( 0 != ret )
    {
        log_error( "read ifcfgFile fail" );
        return 0;
    }

    log_debug("ifcfg info: [%s]", ifcfginfo.c_str());
    jstring jifcfginfo = env->NewStringUTF( ifcfginfo.c_str() );
    if ( NULL == jifcfginfo )
    {
        if (env->ExceptionCheck()) {
            log_error( "NewStringUTF fail" );
        }
    }
    return jifcfginfo;
}

jint JNICALL Java_com_bluedon_core_jni_DevJNI_modifyIfcfgFile
( JNIEnv* env, jobject obj, jstring jbasepath, jstring jifcfginfo )
{
    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )
        return -1;

    const char* ifcfginfo = env->GetStringUTFChars( jifcfginfo, 0 );
    if ( 0 == ifcfginfo )
    {
        log_error( "GetStringUTFChars return NULL" );
        return -1;
    }

    log_debug( "modifyIfcfgInfo: [%s]", ifcfginfo );

    int ret = bd_network_modifyIfcfg( ifcfginfo );
    if ( 0 != ret )
    {
        log_error( "bd_network_modifyIfcfg fail" );
        env->ReleaseStringUTFChars( jifcfginfo, ifcfginfo );
        return -1;
    }

    string device;
    string buf;
    bd_split( ifcfginfo, ",", &device, &buf );
    buf = "ifdown " + device + ";ifup " + device;

    log_info("begin execute %s", buf.c_str());
    bd_action_local local( buf, "" );
    ret = local.execute(); // 因为执行ifdown/ifup时,如果此网卡没接网线,system()函数返回256,如果接网线,返回0,所以忽略返回结果ret
    log_info("end execute %s", buf.c_str());

    env->ReleaseStringUTFChars( jifcfginfo, ifcfginfo );

    return 0;
}

jstring JNICALL Java_com_bluedon_core_jni_DevJNI_GeoIPRegion
( JNIEnv* env, jobject obj, jstring jip)
{

    if ( 0 != bd_jni_init::instance()->init(BD_SOCSERVER_BASEPATH) )
        return 0;

    GeoIP *gi = NULL;
    gi = GeoIP_open(DATAFILE, GEOIP_MEMORY_CACHE);
    if (gi == NULL) {
        log_error("GeoIP_open %s failed, please check!", DATAFILE);
        return 0;
    }

    const char* ip = env->GetStringUTFChars( jip, 0 );

    GeoIPRecord *gir = GeoIP_record_by_name(gi, ip);

    if (gir == NULL) {
        GeoIP_delete(gi);
        env->ReleaseStringUTFChars( jip, ip );
        log_error("GeoIP_record_by_name failed, please check!");
        return 0;
    }

    //data:country~region~city~latitude~longitude
    string data;
    data.clear();
    if (gir->country_name != NULL) {
        data = gir->country_name;
    }
    data += "~";
    const char* region_name = GeoIP_region_name_by_code(gir->country_code, gir->region);
    if (region_name != NULL) {
        data += region_name;
    }
    //data += GeoIP_region_name_by_code(gir->country_code, gir->region);
    data += "~";
    if (gir->city != NULL) {
        data += gir->city;
    }
    data += "~";

    char latitude[20];
    memset(latitude, 0, sizeof(latitude));
    gcvt(gir->latitude, 6, latitude);
    //data += gir->latitude;
    data += latitude;

    data += "~";

    char longitude[20];
    memset(longitude, 0, sizeof(longitude));
    gcvt(gir->longitude, 6, longitude);

    data += longitude;

    log_debug("this report data is : %s", data.c_str());

    jstring jdata = env->NewStringUTF( data.c_str() );

    env->ReleaseStringUTFChars( jip, ip );

    if (gir != NULL) {
        GeoIPRecord_delete(gir);
    }

    if (gi != NULL) {
        GeoIP_delete(gi);
    }

    return jdata;
}

jint JNICALL Java_com_bluedon_core_jni_DevJNI_buzzerControl
( JNIEnv* env, jobject obj, jint jmode )
{
    log_info("jni_DevJNI_buzzerControl begin...!");
    
    if(jmode)
    {
        int re = bd_buzzer_start();
        if(re != 0) return -1;
    } else
    {
        bd_buzzer_stop();
    }
    
    log_info("jni_DevJNI_buzzerControl end...!");
    
    return 0;
}
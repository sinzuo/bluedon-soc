/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_servicestatus.h
 * @brief   soc所在的主机及相关后台服务的控制操作, snmp,syslog-ng,tomcat,日志采集器及soc
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef BD_SERVICESTATUS_H_
#define BD_SERVICESTATUS_H_

#include <string>
using namespace std;

/*
 *  获取soc相关后台服务的运行状态
 *  @param serviceName      服务的名称
 *  @return                 0:正常,1:停止
 */
//int bd_get_normalServiceStatus( const string& serviceName );

/*
 *  获取soc相关后台服务的运行状态
 *  @param serviceName      服务的名称
 *  @return                 0:成功,-1:失败
 */
//int bd_get_servicesStatus( const string& command, string* status );

/*
 *  在Linux中,用service命令重启后台服务
 *  @param servicename      服务的名称
 *  @return                 0:成功,-1:失败
 */
//int bd_restart_by_linuxservicecommand(string const& servicename);

/*
 *  获取soc相关后台服务的进程id
 *  @param serviceName      服务的名称
 *  @return                 -1:失败, 其它:进程id
 */
//int bd_get_servicePid( const string& command );

/*
 *  重启soc相关的后台服务
 *  @param serviceName      服务的名称
 *  @return                 0:成功,-1:失败
 */
//int bd_restart_service( const std::string& command );

/*
 *  shutdown soc所在的主机
 *  @param 无
 *  @return                 0:成功,-1:失败
 */
//int bd_shutdown_host();

/*
 *  reboot soc所在的主机
 *  @param 无
 *  @return                 0:成功,-1:失败
 */
//int bd_reboot_host();

/*
 *  设置 soc所在主机的系统时间
 *  @param setTime          需设置成的时间,格式为"YYYY-MM-DD HH:mm:SS"
 *  @return                 0:成功,-1:失败
 */
int bd_set_time(const string& setTime);

#endif // BD_SERVICESTATUS_H_


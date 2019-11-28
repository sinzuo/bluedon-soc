#ifndef __TOMCAT_MONITOR_MODULE_H__
#define __TOMCAT_MONITOR_MODULE_H__
/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file   tomcat_monitor_module.h
 * @brief   monitor类
 * 获取系统cpu mem load等指标信息
 *
 * @version 1.0
 * @author  bdsoc
 * @date    2017年9月11日
 *
 * 修订说明：最初版本
 **************************************************/

#include <string>
#include <iomanip>

#include "bd_monitor_base.h"

class TomcatMonitorModule : public BDMonitorBase
{
public:
    TomcatMonitorModule();
    TomcatMonitorModule(const string &ip, int port, const string &cmd, const string &authinfo);

    virtual int collect();
    virtual string Output();

private:
    int parserHtml(const string &source);
    void initParameters();
    string correctToXML(const string &content);

private:
    string m_basicinfo;
    string m_threadinfo;
    string m_jvminfo;
    string m_operainfo;
};

#endif // __TOMCAT_MONITOR_MODULE_H__

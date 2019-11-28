#ifndef __APACHE_MONITOR_MODULE_H__
#define __APACHE_MONITOR_MODULE_H__
/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file   apache_monitor_module.h
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

class ApacheMonitorModule : public BDMonitorBase
{
public:
    ApacheMonitorModule();
    ApacheMonitorModule(const string &ip, int port, const string &cmd, const string &authinfo);

    virtual int collect();
    virtual string Output();

private:
    void initParameters();
    int parserHtml(const string &source);

private:
    string m_basicinfo;
    string m_statusinfo;
};

#endif // APACHE_MONITOR_MODULE_H

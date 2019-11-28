#ifndef __BD_MONITOR_BASE_H__
#define __BD_MONITOR_BASE_H__

/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file   bd_monitor_base.h
 * @brief   monitor基类
 * 获取系统cpu mem load等指标信息
 *
 * @version 1.0
 * @author  bdsoc
 * @date    2017年9月11日
 *
 * 修订说明：最初版本
 **************************************************/

#include <iostream>
#include <string>

#define LEN_128     128
#define LEN_512     512
#define LEN_1024    1024
#define LEN_4096    4096

using namespace std;

class BDMonitorBase
{
public:
    BDMonitorBase();

    void setInterval(int interval) { _interval = interval; }
    int getInterval() { return _interval; }

    void setIP(const string &ip) { _ip = ip; }
    string getIP(){ return _ip; }

    void setPort(int port) { _port = port; }
    int getPort() { return _port; }

    void setModName(const string &name) { _modName = name; }
    string getModName() { return _modName; }

    int getItem() { return _policyItem; }
    void setItem(int policyItem) { _policyItem = policyItem; }

    void setAuthInfo(const string &authinfo) { _authinfo = authinfo; }
    string getAuthInfo() const { return _authinfo; }

    void setCmd(const string &cmd) { _cmd = cmd; }
    string getCmd() { return _cmd; }

    string getErrorReason() {return _errdesc;}

    string createHttpHead(const string &cmd, const string &authinfo, const string &ip, int port);

public: //虚函数部分
    virtual ~BDMonitorBase(){}

    virtual int collect() = 0; //纯虚函数
    virtual void initOption() {}
    virtual string Output(){ return ""; } // = 0;

protected:
    int     _interval; //时间间隔
    string  _filename;
    string  _ip;
    int     _port;
    string  _modName;
    int     _policyItem;

    string  _authinfo;
    string  _cmd;

    int     _errcode;
    string  _errdesc;
};


#endif // __BD_MONITOR_BASE_H__

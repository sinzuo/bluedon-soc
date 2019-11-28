/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    xxx.h
 * @brief   xxx
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#if 0
#ifndef BD_JNI_UPDATENELOGININFO_H_
#define BD_JNI_UPDATENELOGININFO_H_


#include <string>
#include <list>
using namespace std;
#include "bd_common.h"

struct _ne_logininfo {
    string ip;
    int port;
    string protocol;
    string user;

    string pass;
    int recordid;

    _ne_logininfo(): port(0), recordid(0) {};
    ~_ne_logininfo() {};
    _ne_logininfo(const _ne_logininfo& other) {
        ip = other.ip;
        port = other.port;
        protocol = other.protocol;
        user = other.user;
        pass = other.pass;
        recordid = other.recordid;
    };

    _ne_logininfo& operator=(const _ne_logininfo& other) {
        ip = other.ip;
        port = other.port;
        protocol = other.protocol;
        user = other.user;
        pass = other.pass;
        recordid = other.recordid;
        return *this;
    };

    bool operator==(const _ne_logininfo& other) {
        if (ip == other.ip &&
            port == other.port &&
            0 == bd_comparenocase(protocol, other.protocol) &&
            user == other.user) {
            return true;
        }

        return false;
    };

    bool operator<(const _ne_logininfo& other) {
        if (ip >= other.ip) {
            return false;
        }

        if (port >= other.port) {
            return false;
        }

        if (bd_comparenocase(protocol, other.protocol) >= 0) {
            return false;
        }

        if (user >= other.user) {
            return false;
        }

        return true;
    };

    bool operator<(const _ne_logininfo& other) const {
        if (ip >= other.ip) {
            return false;
        }

        if (port >= other.port) {
            return false;
        }

        if (bd_comparenocase(protocol, other.protocol) >= 0) {
            return false;
        }

        if (user >= other.user) {
            return false;
        }

        return true;
    };
};

class bd_nelogininfo {
private:
    int _get_nelogininfo_from_db();
    int _get_nelogininfo_from_remote();
    int _update_nelogininfo();

public:
    int update_nelogininfo();

private:
    list<_ne_logininfo> m_dbnelogininfo;
    list<_ne_logininfo> m_remotenelogininfo;
};

#endif // BD_JNI_UPDATENELOGININFO_H_

#endif

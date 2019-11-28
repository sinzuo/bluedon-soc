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
#include "bd_jni_updatenelogininfo.h"
#include <string>
#include <set>
#include <algorithm>
using namespace std;
#include "ace/OS.h"
#include "bd_log_macros.h"
#include "bd_db_command.h"
#include "bd_https_control.h"
#include "bd_conf_ctl.h"
#include "bd_file.h"
#include "bd_crypto_wrap.h"
#include "bd_https_control.h"

int bd_nelogininfo::_get_nelogininfo_from_db() {
    BD_TRACE(bd_nelogininfo, _get_nelogininfo_from_db);

    // snmp协议除外
    char const *querystr =
        "SELECT tcip.ipv4, tgtcnl.tdname, tcnl.port, tcnl.username, tcnl.password, tcnl.recordid "
        "FROM TC_NE_LOGIN tcnl LEFT JOIN TC_IPV4 tcip ON tcnl.ipv4id = tcip.recordid "
        "LEFT JOIN TD_GENERAL tgtcnl ON tcnl.protid = tgtcnl.recordid "
        "LEFT JOIN TC_NE tcne ON tcnl.neid = tcne.recordid "
        "WHERE tcip.status = 1 AND tcnl.status = 1 AND tgtcnl.tdname != 'SNMP' AND tcne.status = 1;";
    BD_LOGTRC("sql:[%s]", querystr);
    bd_db_command cmd(querystr);

    if (!cmd.hasNext()) {
        BD_LOGERR("result is NULL");
        return -1;
    }

    while (cmd.hasNext()) {
        _ne_logininfo info;
        string user, pass;
        cmd >> info.ip >> info.protocol >> info.port >> user >> pass >> info.recordid;

        char* userplain = 0;
        int userlen = 0;
        char* passplain = 0;
        int passlen = 0;

        if (bd_tdesStringDecrypt(user.c_str(), user.size(), userplain, &userlen, 0) ||
            bd_tdesStringDecrypt(pass.c_str(), pass.size(), passplain, &passlen, 0)) {
            BD_LOGERR("fail to decrypt, user:%s, pass:%s", user.c_str(), pass.c_str());
            delete[] userplain;
            delete[] passplain;
            continue;
        }

        info.user.assign(userplain, userlen);
        info.pass.assign(passplain, passlen);
        m_dbnelogininfo.push_back(info);
        delete[] userplain;
        delete[] passplain;
        BD_LOGTRC("ip:%s, port:%d, protocol:%s, user:%s, pass:%s, recordid:%d",
                  info.ip.c_str(), info.port, info.protocol.c_str(), info.user.c_str(), info.pass.c_str(), info.recordid);
    }

    return 0;
}

int bd_nelogininfo::_get_nelogininfo_from_remote() {
    BD_TRACE(bd_nelogininfo, _get_nelogininfo_from_remote);

    if (! bd_conf_ctl::instance()->isMapExist("BLJ")) {
        string path;

        if (0 != bd_conf_ctl::instance()->getConffilePath("BLJCONFFILE", &path)) {
            BD_LOGERR("get bljlogininfo conf path fail");
            return -1;
        }

        if (0 != bd_conf_ctl::instance()->initNoCrypto("BLJ", path)) {
            BD_LOGERR("init serial conf map fail");
            return -1;
        }
    }

    string hostip, user, pass, port, certfile;

    if (bd_conf_ctl::instance()->get("BLJ", "hostip", &hostip) ||
        bd_conf_ctl::instance()->get("BLJ", "user", &user) ||
        bd_conf_ctl::instance()->get("BLJ", "pwd", &pass) ||
        bd_conf_ctl::instance()->get("BLJ", "port", &port) ||
        bd_conf_ctl::instance()->get("BLJ", "certfile", &certfile)) {
        BD_LOGERR("get bljlogininfo conf fail");
        return -1;
    }

    string addrtmp("https://");
    addrtmp += hostip + ":" + port + "/bdsec";
    bd_https_control httpsctl(addrtmp); //输入bdsec的地址
    httpsctl.authentication(user, pass);    //用户名密码
    httpsctl.set_verify_file(certfile); //证书地址

    string iplist;

    for (list<_ne_logininfo>::iterator it = m_dbnelogininfo.begin();
         it != m_dbnelogininfo.end(); ++it) {
        if (!iplist.empty()) {
            iplist += ",";
        }

        iplist += it->ip;
    }

    list<NELogin*> nelogin;
    httpsctl.getNELogin(iplist, &nelogin);

    for (list<NELogin*>::iterator loginit = nelogin.begin();
         loginit != nelogin.end(); ++loginit) {
        _ne_logininfo info;
        info.ip = (*loginit)->host.Ip;
        info.port = (*loginit)->host.Port;
        info.protocol = (*loginit)->ProtoType;
        info.user = (*loginit)->user.Name;
        info.pass = (*loginit)->user.Password;
        m_remotenelogininfo.push_back(info);
        delete(*loginit);
        BD_LOGTRC("ip:%s, port:%d, protocol:%s, user:%s, pass:%s",
                  info.ip.c_str(), info.port, info.protocol.c_str(), info.user.c_str(), info.pass.c_str());
    }

    return 0;
}

int bd_nelogininfo::_update_nelogininfo() {
    BD_TRACE(bd_nelogininfo, _update_nelogininfo);

    char querystr[2048] = {0};
    bd_db_command cmd;
    cmd.setAutoCommit(false);

    try {
        for (list<_ne_logininfo>::iterator it = m_dbnelogininfo.begin(); it != m_dbnelogininfo.end(); ++it) {
            list<_ne_logininfo>::iterator iter = find(m_remotenelogininfo.begin(), m_remotenelogininfo.end(), *it);

            if (m_remotenelogininfo.end() == iter) {
                BD_LOGTRC("remote not exist");
                continue;
            }

            //exist
            int dstSize = 0;
            char* pass = 0;
            pass[dstSize] = '\0';

            if (bd_tdesStringEncrypt(iter->pass.c_str(), iter->pass.size(), pass, &dstSize, 0)) {
                BD_LOGERR("fail to encrypt, user:%s, pass:%s", iter->user.c_str(), iter->pass.c_str());
                continue;
            }

            ACE_OS::snprintf(querystr, sizeof(querystr),
                             "UPDATE TC_NE_LOGIN SET password = '%s' WHERE recordid = %d", pass, it->recordid);
            BD_LOGTRC("sql:[%s]", querystr);
            delete[] pass;
            cmd.execute(querystr);
            m_remotenelogininfo.erase(iter);
        }

        cmd.commit();

    } catch (const bd_db_exception& e) {
        BD_LOGERR("----------- DB ERROR, EXCEPTION INFO -----------");
        BD_LOGERR("异常信息: [%s]", e.what());
        BD_LOGERR("异常代码: [%d]", e.getcode());
        BD_LOGERR("异常级别: [%d]", e.getlevel());
        BD_LOGERR("------------------------------------------------");
        return -1;
    }

    return 0;
}

int bd_nelogininfo::update_nelogininfo() {
    BD_TRACE(bd_nelogininfo, update_nelogininfo);

    try {
        if (0 != _get_nelogininfo_from_db()) {
            BD_LOGERR("fail to get ne login info from db");
            return -1;
        }

        if (0 != _get_nelogininfo_from_remote()) {
            BD_LOGERR("fail to get ne login info from remote");
            return -1;
        }

        _update_nelogininfo();

    } catch (const bd_db_exception& e) {
        BD_LOGERR("----------- DB ERROR, EXCEPTION INFO -----------");
        BD_LOGERR("异常信息: [%s]", e.what());
        BD_LOGERR("异常代码: [%d]", e.getcode());
        BD_LOGERR("异常级别: [%d]", e.getlevel());
        BD_LOGERR("------------------------------------------------");
        return -1;

    } catch (const bd_net_exception& e) {
        BD_LOGTRC("CATCH bd_net_exception");
        BD_LOGERR("bdsec getNELogin fail", e.what());
        return -1;

    } catch (...) {
        BD_LOGERR("unknown error");
        return -1;
    }

    return 0;
}
#endif

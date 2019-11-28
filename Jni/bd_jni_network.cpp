/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_jni_network.cpp
 * @brief   获取/修改关于网络的配置文件,/etc/sysconfig/network-scripts/ifcfg-eth*, /etc/resolv.conf
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年11月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "bd_jni_network.h"
#include <string>
#include <list>
#include <map>
#include <algorithm>
#include <fstream>
using namespace std;
#include "ace/OS.h"
#include "bd_jni.h"
#include "utils/bd_common.h"


int bd_network_getIfcfgFilesPath(const string& dirName, list<string>* ifcfgFilesName);
int bd_network_selector(const struct ACE_DIRENT* dir);
int bd_network_readIfcfgInfo(const string& ifcfgEthFile, map<string, string>* ifcfgMap);
int bd_network_initMap(const string& path, map<string, string>* confName);
int bd_network_getFromMap(const map<string, string>& ifcfgMap, const string& ifcfgKey, string* ifcfgValue);

int bd_network_getModityIfcfgInfo(const string& ifcfgInfoStr, map<string, string>* ifcfgMap);
int bd_network_readModityIfcfgFile(const map<string, string>& ifcfgMap, string* ifcfgPath);
int bd_network_modifyIfcfgFile(const string& ifcfgPath, map<string, string>& ifcfgMap);

int bd_makeup_path(const string& dir, const string& filename, string* path);

int bd_network_readIfcfg(string* ifcfgInfo, const string& dirName) {
    //BD_TRACE( bd_jni_network, bd_network_readIfcfg );

    if (dirName.empty() || 0 == ifcfgInfo) {
        log_error("arg:dirName/ifcfgInfo is empty");
        return -1;
    }

    list<string> ifcfgFilesName;

    if (0 != bd_network_getIfcfgFilesPath(dirName, &ifcfgFilesName)) {
        log_error("bd_network_getIfcfgFilesPath fail");
        return -1;
    }

    ifcfgFilesName.sort();

    for (list<string>::iterator iter = ifcfgFilesName.begin(); iter != ifcfgFilesName.end(); ++iter) {
        log_debug("get ifcfg info from [%s]", iter->c_str());
        map<string, string> ifcfgMap;
        bd_ifcfgInfo ifcfg;

        if (0 != bd_network_readIfcfgInfo(*iter, &ifcfgMap)) {
            log_error("read [%s] fail", iter->c_str());
            continue; //如果链表中的某个ifg-eth文件读取失败,跳过,继续读未读取的文件
        }

        if (!ifcfgInfo->empty()) {
            ifcfgInfo->append(";");   //末尾不用加";"
        }

        // "DEVNAME,IPADDR,NETMASK,GATEWAY,ONBOOT,DNS1,DNS12[;...]"
        bd_network_getFromMap(ifcfgMap, "DEVICE", &(ifcfg.devName));
        bd_network_getFromMap(ifcfgMap, "IPADDR", &(ifcfg.IPAddr));
        bd_network_getFromMap(ifcfgMap, "NETMASK", &(ifcfg.netMask));
        bd_network_getFromMap(ifcfgMap, "GATEWAY", &(ifcfg.gateWay));
        bd_network_getFromMap(ifcfgMap, "ONBOOT", &(ifcfg.onBoot));
        bd_network_getFromMap(ifcfgMap, "DNS1", &(ifcfg.dns1));
        bd_network_getFromMap(ifcfgMap, "DNS2", &(ifcfg.dns2));
        ifcfgInfo->append(ifcfg.devName);
        ifcfgInfo->append(",");
        ifcfgInfo->append(ifcfg.IPAddr);
        ifcfgInfo->append(",");
        ifcfgInfo->append(ifcfg.netMask);
        ifcfgInfo->append(",");
        ifcfgInfo->append(ifcfg.gateWay);
        ifcfgInfo->append(",");
        ifcfgInfo->append(ifcfg.onBoot);
        ifcfgInfo->append(",");
        ifcfgInfo->append(ifcfg.dns1);
        ifcfgInfo->append(",");
        ifcfgInfo->append(ifcfg.dns2);
        log_debug("ifcfg info [%s]", ifcfgInfo->c_str());
    }

    return 0;
}

int bd_network_getIfcfgFilesPath(const string& dirName, list<string>* ifcfgFilesName) {
    //BD_TRACE( bd_jni_network, bd_network_getIfcfgFilesPath );

    if (dirName.empty() || 0 == ifcfgFilesName) {
        log_error("arg:dirName/ifcfgFilesName is empty");
        return -1;
    }

    struct ACE_DIRENT **namelist;

    int n;

    n = ACE_OS::scandir(dirName.c_str(), &namelist, bd_network_selector, (ACE_SCANDIR_COMPARATOR)ACE_OS::alphasort);

    if (n < 0) {
        log_error("Failed to open dir %s", dirName.c_str());
        return -1;
    }

    //ifcfgFilesName->clear();
    while (n--) {
        string path_name;

        if (0 != bd_makeup_path(dirName, namelist[n]->d_name, &path_name)) {
            log_error("make up path fail, dir [%s], file name [%s]", dirName.c_str(), namelist[n]->d_name);
            return -1;
        }

        ifcfgFilesName->push_back(path_name);
        ACE_OS::free(namelist[n]);
    }

    if (ifcfgFilesName->empty()) {
        log_error("no ifcfg-eth file, dir is : %s", dirName.c_str());
        return -1;
    }

    ACE_OS::free(namelist);

    return 0;
}
int bd_makeup_path(const string& dir, const string& filename, string* path) {
    if (dir.empty() || filename.empty() || 0 == path) {
        return -1;
    }

    *path = dir;

    if (path->at(path->length() - 1) != '/') {
        path->append("/");
    }

    if (filename.at(0) == '/') {
        path->append(filename, 1, filename.length() - 1);

    } else {
        path->append(filename);
    }

    return 0;
}

bool bd_network_check_ethname(const string& ethname) {
    //对centos7 网卡的兼容
	if (!memcmp(ethname.c_str(), "enp", 3) || !memcmp(ethname.c_str(), "ens", 3)) {
        log_error("ethname [%s] is wrong", ethname.c_str());
        return true;
    }
    else  if (ACE_OS::strncasecmp(ethname.c_str(), "eth", 3)) {
        log_error("ethname [%s] is wrong", ethname.c_str());
        return false;
    }

    for (string::const_iterator it = ethname.begin() + 3; ethname.end() != it; ++it) {
        if (! isdigit(*it)) {
            log_error("ethname [%s] is wrong", ethname.c_str());
            return false;
        }
    }

    return true;
}

int bd_network_selector(const struct ACE_DIRENT* dir) {
		
//    //对centos7 网卡的兼容
//    // file name : ifcfg-enp1s0, ifcfg-ens1s0 ...
//    if (!memcmp(dir->d_name, "ifcfg-enp", 9) || !memcmp(dir->d_name, "ifcfg-ens", 9)) {
//        return 1;    //返回非0时,scandir()才把此文件选中
//    }

//    // file name : ifcfg-eth0, ifcfg-eth1 ...
//    else if (!memcmp(dir->d_name, "ifcfg-eth", 9) &&
//        bd_network_check_ethname(dir->d_name + 6)) {
//        return 1;    //返回非0时,scandir()才把此文件选中
//    }
    if(!memcmp(dir->d_name, "ifcfg-", 6)){
        string d_name = dir->d_name;
//        log_error("ifcfg:[%s]",d_name.c_str());
        if(d_name.find("lo") == string::npos && d_name.find(".") == string::npos){
            return 1;
        }
    }

    return 0;
}

int bd_network_readIfcfgInfo(const string& ifcfgEthFile, map<string, string>* ifcfgMap) {
    //BD_TRACE( bd_jni_network, bd_network_readIfcfgInfo );

    if (ifcfgEthFile.empty() || 0 == ifcfgMap) {
        log_error("arg:ifcfgEthFile/ifcfgMap is empty");
        return -1;
    }

    log_debug("get ifcfg info from [%s]", ifcfgEthFile.c_str());
    //ifcfgMap->clear();
    return bd_network_initMap(ifcfgEthFile, ifcfgMap);
}

int bd_network_initMap(const string& path, map<string, string>* confName) {
    //BD_TRACE( bd_jni_network, bd_network_initMap );

    if (path.empty() || 0 == confName) {
        log_error("arg:path/pointer confName is empty");
        return -1;
    }

    log_debug("get ifcfg info from [%s]", path.c_str());
    std::fstream fin(path.c_str(), std::ios::in | std::ios::binary);

    if (!fin) {
        log_error("Failed to open %s!\n", path.c_str());
        return -1;
    }

    while (!fin.eof()) {
        string line;

        if (!getline(fin, line)) {
            log_error("getline fail");
            continue;
        }

        /*if ( line.length() <= 0 )
        {
            continue;
        }
        if ( *(line.begin()) == '\n' || *(line.begin()) == '\r' )
        {
            continue;
        }*/

        bd_trim(&line);

        if (line.empty()) { // 空行
            continue;
        }

        if (line.at(0) == '#') { //注释
            continue;
        }

        string confkey;
        string confvalue;

        if (2 != bd_split(line, "=", &confkey, &confvalue)) {
            log_error("not a conf line, line : %s", line.c_str());
            continue;
        }

        bd_trimright(&confkey);
        bd_trimleft(&confvalue);
        confName->insert(std::pair<string, string>(confkey, confvalue));
        log_debug("key:%s--value:%s", confkey.c_str(), confvalue.c_str());
    }

    log_debug("init conf map success");

    if (confName->empty()) {
        log_error("conf file is null, path : %s", path.c_str());
        fin.close();
        return -1;
    }

    fin.close();
    return 0;
}

int bd_network_getFromMap(const map<string, string>& ifcfgMap, const string& ifcfgKey, string* ifcfgValue) {
    //BD_TRACE( bd_jni_network, bd_network_getFromMap );

    if (ifcfgMap.empty() || ifcfgKey.empty() || 0 == ifcfgValue) {
        log_error("arg:ifcfgMap/ifcfgKey/ifcfgValue is empty");
        return -1;
    }

    map<string, string>::const_iterator iter = ifcfgMap.find(ifcfgKey);

    if (ifcfgMap.end() == iter) {
        log_error("ifcfgKey:[%s] not exist", ifcfgKey.c_str());
        *ifcfgValue = ""; // ifcfg-eth中无此项,即为空

    } else {
        *ifcfgValue = iter->second;
    }

    return 0;
}

int bd_network_modifyIfcfg(const string& ipInfo) {
    //BD_TRACE( bd_jni_network, bd_network_modifyIfcfg );

    if (ipInfo.empty()) {
        log_error("arg ipInfo is empty");
        return -1;
    }

    map<string, string> ifcfgMap;

    if (0 != bd_network_getModityIfcfgInfo(ipInfo, &ifcfgMap)) {
        log_error("get ifcfg info fail, ipInfo : %s", ipInfo.c_str());
        return -1;
    }

    string ifcfgPath;

    if (0 != bd_network_readModityIfcfgFile(ifcfgMap, &ifcfgPath)) {
        log_error("read ifcfg info fail, ifcfg path : %s", ifcfgPath.c_str());
        return -1;
    }

    if (0 != bd_network_modifyIfcfgFile(ifcfgPath, ifcfgMap)) {
        log_error("modify ifcfg [%s] fail", ifcfgPath.c_str());
        return -1;
    }

    return 0;
}

// ifcfgInfoStr : "devName,IPAddr,netMask,gateWay,onBoot,dns1,dns2"
// 必须为"key=value"的形式，且以","隔开，key必须大写
int bd_network_getModityIfcfgInfo(const string& ifcfgInfoStr, map<string, string>* ifcfgMap) {
    //BD_TRACE( bd_jni_network, bd_network_getModityIfcfgInfo );

    if (ifcfgInfoStr.empty() || 0 == ifcfgMap) {
        log_error("arg:ifcfgInfoStr/pointer ifcfgMap is empty");
        return -1;
    }

    std::vector<string> ifcfgKeyVector;
    ifcfgKeyVector.push_back("DEVICE");
    ifcfgKeyVector.push_back("IPADDR");
    ifcfgKeyVector.push_back("NETMASK");
    ifcfgKeyVector.push_back("GATEWAY");
    ifcfgKeyVector.push_back("ONBOOT");
    ifcfgKeyVector.push_back("DNS1");
    ifcfgKeyVector.push_back("DNS2");
    list<string> ipInfo;
    bd_split(ifcfgInfoStr, ",", &ipInfo);
    std::vector<string>::iterator it = ifcfgKeyVector.begin();
    list<string>::iterator iter = ipInfo.begin();

    for (;
         it != ifcfgKeyVector.end(), iter != ipInfo.end();
         ++it, ++iter) {
        string ifcfgValue = *iter;
        bd_trim(&ifcfgValue);
        log_debug("value befor:%s, end:%s", iter->c_str(), ifcfgValue.c_str());
        ifcfgMap->insert(make_pair(*it, ifcfgValue));
    }

    map<string, string>::iterator mapIter = ifcfgMap->find("IPADDR");

    if (ifcfgMap->end() != mapIter) {
        if (mapIter->second.empty()) {
            log_debug("IPADDR is empty, BOOTPROTO set dhcp");
            ifcfgMap->insert(std::make_pair("BOOTPROTO", "dhcp"));

        } else {
            //如果需修改的IP信息中给某个网卡赋了IP地址，那么修改启动协议为不使用dhcp协议
            log_debug("IPADDR not empty, BOOTPROTO set none");
            ifcfgMap->insert(std::make_pair("BOOTPROTO", "none"));
        }
    }

    return 0;
}

int bd_network_readModityIfcfgFile(const map<string, string>& ifcfgMap, string* ifcfgPath) {
    //根据修改的网卡名判断需要对哪个配置文件进行修改
    //BD_TRACE( bd_jni_network, bd_network_readModityIfcfgFile );

    if (ifcfgMap.empty() || 0 == ifcfgPath) {
        log_error("arg:ifcfgMap/ifcfgPath is empty");
        return -1;
    }

    string devName;
    map<string, string>::const_iterator iter = ifcfgMap.find("DEVICE");

    if (ifcfgMap.end() == iter) {
        log_error("no DEVICE");
        return -1;
    }

    devName = iter->second;

    if (devName.empty()) {
        log_error("DEVICE is empty");
        return -1;
    }

    if (! bd_network_check_ethname(devName)) {
        log_error("devName [%s] is wrong", devName.c_str());
        return -1;
    }
    /*
    string ifcfgTemp = "/etc/sysconfig/network-scripts/ifcfg-eth";
    string ifcfgNum = devName.substr(3);
    ifcfgTemp.append(ifcfgNum);
    */
	string ifcfgTemp = "/etc/sysconfig/network-scripts/ifcfg-";
    ifcfgTemp.append(devName);
    if (ACE_OS::access(ifcfgTemp.c_str(), F_OK) != 0) {
        log_error("ifcfg file for [%s] not exist", ifcfgTemp.c_str());
        return -1;
    }

    *ifcfgPath = ifcfgTemp;
    log_debug("ifcfg full path name [%s]", ifcfgPath->c_str());
    return 0;
}

int bd_network_modifyIfcfgFile(const string& ifcfgPath, map<string, string>& ifcfgMap) {
    //BD_TRACE( bd_jni_network, bd_network_modifyIfcfgFile );

    if (ifcfgPath.empty() || ifcfgMap.empty()) {
        log_error("ifcfg conf file name is empty");
        return -1;
    }

    std::fstream fin(ifcfgPath.c_str(), ios::in | ios::binary);

    if (!fin) {
        log_error("Failed to open %s!\n", ifcfgPath.c_str());
        return -1;
    }

    string ifcfgPathTemp = ifcfgPath + ".tmp";
    std::fstream fout(ifcfgPathTemp.c_str(), ios::out | ios::binary);

    if (!fout) {
        fout.close();
        log_error("Failed to open %s!\n", ifcfgPathTemp.c_str());
        return -1;
    }

    while (!fin.eof()) {
        string line;

        if (!getline(fin, line)) {
            log_error("getline fail");
            continue; // break?
        }

        string temp(line);
        bd_trim(&temp);

        if (temp.empty()) { // 空行
            continue;
        }

        if (temp.at(0) == '#') { // 注释
            fout.write(line.c_str(), line.length());
            fout.write("\n", 1);
            continue;
        }

        // 非注释,非配置行
        string ifcfgKey;
        string ifcfgValue;

        if (2 != bd_split(line, "=", &ifcfgKey, &ifcfgValue)) {
            fout.write(line.c_str(), line.length());
            fout.write("\n", 1);
            continue;
        }

        bd_trim(&ifcfgKey);
        map<string, string>::iterator iter = ifcfgMap.find(ifcfgKey);

        // 如果配置文件中的配置项没有出现在参数中,则无须修改该项
        if (ifcfgMap.end() == iter) {
            log_debug("[%s] not modify", line.c_str());
            fout.write(line.c_str(), line.length());
            fout.write("\n", 1);
            continue;
        }

        log_debug("after not modify");

        if (iter->second.empty()) {
            log_debug("delete [%s]", line.c_str());
            continue;
        }

        log_debug("after delete");
        fout.write(iter->first.c_str(), iter->first.length());
        fout.write("=", 1);
        fout.write(iter->second.c_str(), iter->second.length());
        fout.write("\n", 1);
        ifcfgMap.erase(iter);   ///////////////////?
    }

    for (map<string, string>::iterator iter = ifcfgMap.begin(); iter != ifcfgMap.end(); ++iter) {
        if (ifcfgMap.end() != iter && !iter->second.empty()) {
            fout.write(iter->first.c_str(), iter->first.length());
            fout.write("=", 1);
            fout.write(iter->second.c_str(), iter->second.length());
            fout.write("\n", 1);
        }
    }

    log_debug("finally write");
    fin.close();
    fout.close();

    log_debug("after close, mv file");

    if (ACE_OS::unlink(ifcfgPath.c_str()) != 0) {
        log_error("remove file [%s] fail", ifcfgPath.c_str());
        return -1;
    }

    if (ACE_OS::rename(ifcfgPathTemp.c_str(), ifcfgPath.c_str())) {
        log_error("rename file [%s] to [%s] fail", ifcfgPathTemp.c_str(), ifcfgPath.c_str());
        return -1;
    }

    return 0;
}




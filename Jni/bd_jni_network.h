/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_jni_network.h
 * @brief   获取/修改关于网络的配置文件,/etc/sysconfig/network-scripts/ifcfg-eth*, /etc/resolv.conf
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年11月13日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef BD_NETWORK_H_
#define BD_NETWORK_H_

#include <string>
using namespace std;


/*
 * ifcfg-eth配置信息
 *
 */
struct bd_ifcfgInfo {
    std::string devName;    ///< netcard name
    std::string IPAddr;     ///< IP地址
    std::string netMask;    ///< 子网掩码
    std::string gateWay;    ///< 网关
    std::string bootProto;  ///< 引导时协议[none|static|bootp|dhcp](不使用|静态分配|BOOTP协议|DHCP协议)
    std::string onBoot;     ///< 引导时是否激活[yes|no]
    std::string dns1;       ///< dns1
    std::string dns2;       ///< dns2

    bd_ifcfgInfo() {}
    ~bd_ifcfgInfo() {};
    bd_ifcfgInfo(const bd_ifcfgInfo& ifcfg) {
        *this = ifcfg;
    }

    bd_ifcfgInfo& operator=(const bd_ifcfgInfo& ifcfg) {
        this->devName = ifcfg.devName;
        this->IPAddr = ifcfg.IPAddr;
        this->netMask = ifcfg.netMask;
        this->gateWay = ifcfg.gateWay;
        this->bootProto = ifcfg.bootProto;
        this->onBoot = ifcfg.onBoot;
        this->dns1 = ifcfg.dns1;
        this->dns2 = ifcfg.dns2;
        return *this;
    }
};

/*
 *  读取配置文件 ifcfg-eth* 的信息
 *  @param ifcfgInfo    读出的配置信息
 *  @param dirName      配置文件所在的目录
 *  @return             0: 成功 -1:失败
 */
int bd_network_readIfcfg(std::string* ifcfgInfo, const std::string& dirName = "/etc/sysconfig/network-scripts");

/*
 *  修改配置文件 ifcfg-eth* 的信息
 *  @param ipInfo   需修改的信息,组成字符串
 *  @return         0: 成功 -1:失败
 */
int bd_network_modifyIfcfg(const std::string& ipInfo);

/*
 *  读取配置文件 /etc/resolv.conf 的DNS
 *  @param ifcfgInfo    读出的DNS信息
 *  @param dirName      配置文件所在的目录
 *  @return             0: 成功 -1:失败
 */
int bd_network_readResolv(std::string* resolvInfo, const std::string& resolvPath = "/etc/resolv.conf");

/*
 *  修改配置文件 /etc/resolv.conf 的 DNS
 *  @param dnsStr   需修改成的 DNS
 *  @param dirName  配置文件所在的目录
 *  @return         0: 成功 -1:失败
 */
int bd_network_modifyResolv(const std::string& dnsStr, const std::string& resolvPath = "/etc/resolv.conf");

/*
 *  添加 DNS 到配置文件 /etc/resolv.conf
 *  @param dnsStr   需添加的 DNS
 *  @param dirName  配置文件所在的目录
 *  @return         0: 成功 -1:失败
 */
int bd_network_addResolv(const std::string& dnsStr, const std::string& resolvPath = "/etc/resolv.conf");

#endif // BD_NETWORK_H_

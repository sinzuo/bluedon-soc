
/** ************************************************
* COPYRIGHT NOTICE
* Copyright (c) 2017,      BLUEDON
* All rights reserved.
*
* @file    BDBaseSysInfo.h
* @brief   传感器
*
*
* @version 5.1
* @author  xxx
* @date    YYYY年MM月DD日
*
* 修订说明：最初版本
* *************************************************/

#ifndef  _BD_BASE_SYSINFO_H_
#define  _BD_BASE_SYSINFO_H_

#include <sys/sysinfo.h>
#include <string.h>

//cpu状态信息结构
typedef struct cpuOccupyEx {
    char name[20];        // name
    unsigned long user;    // user
    unsigned long nice;    // nice
    unsigned long system;  // system
    unsigned long idle;    // idle
    unsigned long iowait;    // iowait
    unsigned long irq;    // irq
    unsigned long softirq;    // softirq

    cpuOccupyEx() {
        memset(name, 0, 20);
    }
} CPU_OCCUPY;


class CBDBaseSysInfo {
private: //singleton
    CBDBaseSysInfo();
    CBDBaseSysInfo(const CBDBaseSysInfo&);
    ~CBDBaseSysInfo();

public:
    inline static CBDBaseSysInfo* instance() {
        return &m_BaseSysInfo;
    }
public:
    int getHostMemInfo(struct sysinfo& tag_meminfo); //获取内存占用
    int getProcCpu(CPU_OCCUPY& cpuinfo);//获取cpu信息
    int calCpuOccupy(const CPU_OCCUPY& cpuinfo_fir, const CPU_OCCUPY& cpuinfo_sec);//计算cpu占用率
    int getDiskOccupy();//获取硬盘占用
private:
    static CBDBaseSysInfo m_BaseSysInfo;    // singleton


};

#endif //_BD_BASE_SYSINFO_H_


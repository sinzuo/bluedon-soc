/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_sysstat.h
 * @brief   获取soc所在主机的资源使用情况,cpu,memory,harddisk
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef BD_PROCINFO_H_
#define BD_PROCINFO_H_

#include <string>
using namespace std;
//#include "bd_common.h"

typedef unsigned long long bd_ulonglong;
/*
 *
 */
struct bd_cpu_usage {
    string cpuName; ///< CPU
    int cpuUsage;   ///< CPU使用率

    bd_cpu_usage(): cpuUsage(0) {};
    ~bd_cpu_usage() {};
    bd_cpu_usage(const bd_cpu_usage& cpu) {
        this->cpuName = cpu.cpuName;
        this->cpuUsage = cpu.cpuUsage;
    };

    bd_cpu_usage& operator=(const bd_cpu_usage& cpu) {
        if (&cpu == this) {
            return *this;
        }

        this->cpuName = cpu.cpuName;
        this->cpuUsage = cpu.cpuUsage;
        return *this;
    };

    bool operator==(const bd_cpu_usage& cpu) {
        if (0 == this->cpuName.compare(cpu.cpuName)) {
            return true;
        }

        return false;
    };
};

struct bd_cpu_time {
    string cpuName;             ///< CPU
    // 从系统启动开始累计到当前时刻,CPU的各种时间, 从/proc/stat 读取
    bd_ulonglong userTime;      ///< 用户态时间,不包含 nice值为负进程
    bd_ulonglong niceTime;      ///< nice值为负的进程所占用的CPU时间
    bd_ulonglong systemTime;    ///< 核心态时间
    bd_ulonglong idleTime;      ///< 除IO等待时间以外的其它等待时间
    bd_ulonglong iowaitTime;    ///< IO等待时间
    bd_ulonglong irqTime;       ///< 硬中断时间
    bd_ulonglong softirqTime;   ///< 软中断时间
    bd_ulonglong stealTime;     ///< which is the time spent in other operating systems when running in a virtualized environment
    bd_ulonglong guestTime;     ///< which is the time spent running a virtual  CPU  for  guest operating systems

    bd_cpu_time():
        userTime(0), niceTime(0), systemTime(0), idleTime(0),
        iowaitTime(0), irqTime(0), softirqTime(0), stealTime(0), guestTime(0) {
    };
    ~bd_cpu_time() {};

    bd_cpu_time(const bd_cpu_time& cpu) {
        this->cpuName = cpu.cpuName;
        this->userTime = cpu.userTime;
        this->niceTime = cpu.niceTime;
        this->systemTime = cpu.systemTime;
        this->idleTime = cpu.idleTime;
        this->iowaitTime = cpu.iowaitTime;
        this->irqTime = cpu.irqTime;
        this->softirqTime = cpu.irqTime;
        this->stealTime = cpu.stealTime;
        this->guestTime = cpu.guestTime;
    };

    bd_cpu_time& operator=(const bd_cpu_time& cpu) {
        if (&cpu == this) {
            return *this;
        }

        this->cpuName = cpu.cpuName;
        this->userTime = cpu.userTime;
        this->niceTime = cpu.niceTime;
        this->systemTime = cpu.systemTime;
        this->idleTime = cpu.idleTime;
        this->iowaitTime = cpu.iowaitTime;
        this->irqTime = cpu.irqTime;
        this->softirqTime = cpu.irqTime;
        this->stealTime = cpu.stealTime;
        this->guestTime = cpu.guestTime;
        return *this;
    };

    bool operator==(const bd_cpu_time& cpu) {
        if (0 == this->cpuName.compare(cpu.cpuName)) {
            return true;

        } else {
            return false;
        }
    };
};

struct bd_mem_usage {
    int memUsage;           ///< memory使用率
    bd_ulonglong memTotal;  ///< memory总容量
    bd_ulonglong memFree;   ///< memory空闲量

    bd_mem_usage(): memUsage(0), memTotal(0), memFree(0) {}
    ~bd_mem_usage() {}
};

struct bd_disk_usage {
    string fsDir;           ///< 文件系统挂载点
    string fsName;          ///< 文件系统名
    int fsUsage;            ///< 文件系统所在分区的使用率
    bd_ulonglong fsTotal;   ///< 分区总大小(KB)
    bd_ulonglong fsFree;    ///< 分区剩余空间(KB)
    bd_disk_usage(): fsUsage(0), fsTotal(0), fsFree(0) {};
    ~bd_disk_usage() {};

    bd_disk_usage(const bd_disk_usage& mem) {
        this->fsName = mem.fsName;
        this->fsDir = mem.fsDir;
        this->fsUsage = mem.fsUsage;
        this->fsTotal = mem.fsTotal;
        this->fsFree = mem.fsFree;
    };

    bd_disk_usage& operator=(const bd_disk_usage& mem) {
        if (&mem == this) {
            return *this;
        }

        this->fsName = mem.fsName;
        this->fsDir = mem.fsDir;
        this->fsUsage = mem.fsUsage;
        this->fsTotal = mem.fsTotal;
        this->fsFree = mem.fsFree;
        return *this;
    };

    bool operator==(const bd_disk_usage& mem) {
        if (0 == this->fsDir.compare(mem.fsDir)) {
            return true;

        } else {
            return false;
        }
    };
};

/*
 *  获取cpu使用情况
 *  @param  无
 *  @return cpu使用率
 */
int bd_get_cpustat();

/*
 *  获取memory使用情况
 *  @param  无
 *  @return memory使用率
 */
int bd_get_memstat();

/*
 *  获取harddisk使用情况
 *  @param  diskstat    harddisk使用状况
 *  @return             0:成功, -1:失败
 */
int bd_get_diskstat(string* diskstat);

#endif // BD_PROCINFO_H_


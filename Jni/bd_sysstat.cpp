/**************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 *
 * @file    bd_sysstat.cpp
 * @brief   获取soc所在主机的资源使用情况,cpu,memory,harddisk
 *
 *
 * @version 1.0
 * @author  llx
 * @date    2011年09月13日
 *
 * 修订说明：最初版本
 **************************************************/

#include "bd_sysstat.h"
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <mntent.h>
#include <sys/vfs.h> // <sys/statsfs.h>
#endif
#include <cstring>
#include <string>
#include <list>
#include <sstream>
using namespace std;
#include "ace/OS.h"
//#include "ace/OS_NS_stdio.h"
#include "utils/bd_common.h"
//#include "bd_log_macros.h"
#include "bd_jni.h"

int bd_get_cpuUsage(list<bd_cpu_usage>* cpu);
int bd_get_cpuTime(list<bd_cpu_time>* cpu);
int bd_cal_cpuUsage(const bd_cpu_time& prev, const bd_cpu_time& curr);

int bd_get_memUsage(bd_mem_usage *mem);

int bd_get_diskUsage(list<bd_disk_usage>* diskUsage);

int bd_get_cpustat() {
    //BD_TRACE( bd_sysstat, bd_get_cpustat );

    list<bd_cpu_usage> cpu;

    if (0 != bd_get_cpuUsage(&cpu)) {
        log_error("get cpu usage fail");
        return -1;
    }

    int cpustat = -1;

    for (list<bd_cpu_usage>::iterator it = cpu.begin();
         it != cpu.end(); ++it) {
        if (it->cpuName != "cpu") {
            continue;
        }

        cpustat = it->cpuUsage;
    }

    return cpustat;
}

int bd_get_cpuUsage(list<bd_cpu_usage>* cpu) {
    //BD_TRACE( bd_sysstat, bd_get_cpuUsage );
    if (0 == cpu) {
        log_error("arg:cpu is NULL");
        return -1;
    }

    list<bd_cpu_time> prev;

    if (0 != bd_get_cpuTime(&prev)) {
        return -1;
    }

    ACE_OS::sleep(3);
    list<bd_cpu_time> curr;

    if (0 != bd_get_cpuTime(&curr)) {
        return -1;
    }

    for (list<bd_cpu_time>::iterator it = prev.begin();
         it != prev.end(); ++it) {
        for (list<bd_cpu_time>::iterator iter = curr.begin();
             iter != curr.end(); ++iter) {
            if (it->cpuName == iter->cpuName) {
                bd_cpu_usage cpuUsage;
                cpuUsage.cpuName = it->cpuName;
                cpuUsage.cpuUsage = bd_cal_cpuUsage(*it, *iter);
                cpu->push_back(cpuUsage);
            }
        }
    }

    return 0;
}

int bd_get_cpuTime(list<bd_cpu_time>* cpu) {
    //BD_TRACE( bd_sysstat, bd_get_cpuTime );
    if (0 == cpu) {
        log_error("arg:cpu is empty");
        return -1;
    }

#ifdef WIN32
    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        log_error("GetSystemTimes fail");
        return -1;
    }

    bd_cpu_time singleCpuTime;
    singleCpuTime.cpuName = "cpu";

    ULARGE_INTEGER temp;
    temp.HighPart = idleTime.dwHighDateTime;
    temp.LowPart = idleTime.dwLowDateTime;
    singleCpuTime.idleTime = temp.QuadPart;

    temp.HighPart = kernelTime.dwHighDateTime;
    temp.LowPart = kernelTime.dwLowDateTime;
    singleCpuTime.systemTime = temp.QuadPart;

    temp.HighPart = userTime.dwHighDateTime;
    temp.LowPart = userTime.dwLowDateTime;
    singleCpuTime.userTime = temp.QuadPart;

    cpu->push_back(singleCpuTime);
#else
    FILE* fd = fopen("/proc/stat", "r");

    if (0 == fd) {
        log_error("open /proc/stat fail");
        return -1;
    }

    char cpuTime[512] = {0};
    char cpuName[16] = {0};
    bd_cpu_time singleCpuTime;

    while (1) {
        memset(cpuName, 0, sizeof(cpuName));
        memset(cpuTime, 0, sizeof(cpuTime));
        fgets(cpuTime, sizeof(cpuTime), fd);

        if (0 == strstr(cpuTime, "cpu")) {
            break;
        }

        //stringstream ss;
        sscanf(cpuTime, "%20s %llu %llu %llu %llu %llu %llu %llu %llu %llu",
               cpuName, &(singleCpuTime.userTime), &(singleCpuTime.niceTime), &(singleCpuTime.systemTime),
               &(singleCpuTime.idleTime), &(singleCpuTime.iowaitTime), &(singleCpuTime.irqTime),
               &(singleCpuTime.softirqTime), &(singleCpuTime.stealTime), &(singleCpuTime.guestTime));
        singleCpuTime.cpuName = cpuName;
        cpu->push_back(singleCpuTime);
    }

    fclose(fd);
#endif

    return 0;
}

int bd_cal_cpuUsage(const bd_cpu_time& prev, const bd_cpu_time& curr) {
    //BD_TRACE( bd_sysstat, bd_cal_cpuUsage );

    bd_ulonglong prevBusyTime = prev.userTime + prev.niceTime + prev.systemTime +
                                prev.iowaitTime + prev.irqTime + prev.softirqTime + prev.stealTime + prev.guestTime;
    bd_ulonglong currBusyTime = curr.userTime + curr.niceTime + curr.systemTime +
                                curr.iowaitTime + curr.irqTime + curr.softirqTime + curr.stealTime + curr.guestTime;
    bd_ulonglong busyTime = currBusyTime - prevBusyTime;
    bd_ulonglong idleTime = curr.idleTime - prev.idleTime;
    log_debug("%s busyTime: %llu, idleTime: %llu", prev.cpuName.c_str(), busyTime, idleTime);
#ifdef WIN32
    return (int)(100 * (busyTime - idleTime) / busyTime);
#else
    return (int)(100 * busyTime / (busyTime + idleTime));
#endif
}

int bd_get_memstat() {
    //BD_TRACE( bd_sysstat, bd_get_memstat );

    bd_mem_usage mem;

    if (0 != bd_get_memUsage(&mem)) {
        log_error("get memory usage fail");
        return -1;
    }

    return mem.memUsage;
}

int bd_get_memUsage(bd_mem_usage *mem) {
    //BD_TRACE( bd_sysstat, bd_get_memUsage );

    if (0 == mem) {
        log_error("arg:mem is empty");
        return -1;
    }

#ifdef WIN32
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);

    if (!GlobalMemoryStatusEx(&memStatus)) {
        log_error("GlobalMemoryStatusEx fail");
        printf("GlobalMemoryStatusEx fail\n");
        return -1;
    }

    mem->memUsage = memStatus.dwMemoryLoad;
    mem->memTotal = memStatus.ullTotalPhys;
    mem->memFree = memStatus.ullAvailPhys;
#else
    FILE* fd = fopen("/proc/meminfo", "r");

    if (0 == fd) {
        log_error("open /proc/meminfo fail");
        return -1;
    }

    char memTotal[256] = {0};
    char memFree[256] = {0};
    char memBuffers[256] = {0};
    char memCached[256] = {0};
    bd_ulonglong memoryFree = 0;
    bd_ulonglong memoryBuffers = 0;
    bd_ulonglong memoryCached = 0;
    char temp[64] = {0};
    fgets(memTotal, sizeof(memTotal), fd);
    fgets(memFree, sizeof(memFree), fd);
    fgets(memBuffers, sizeof(memBuffers), fd);
    fgets(memCached, sizeof(memCached), fd);
    fclose(fd);
    sscanf(memTotal, "%64s %llu", temp, &(mem->memTotal));
    sscanf(memFree, "%64s %llu", temp, &memoryFree);
    sscanf(memBuffers, "%64s %llu", temp, &memoryBuffers);
    sscanf(memCached, "%64s %llu", temp, &memoryCached);
    log_debug("**total:%s %llu, free:%s %llu, buffers:%s %llu, cached:%s %llu",
              memTotal, mem->memTotal, memFree, memoryFree, memBuffers, memoryBuffers, memCached, memoryCached);
    //mem->memFree = memoryFree + memoryBuffers + memoryCached;
    mem->memFree = memoryFree;
    mem->memUsage = 100 - mem->memFree * 100 / mem->memTotal;
#endif

    return 0;
}

int bd_get_diskstat(string* diskstat) {
    //BD_TRACE( bd_sysstat, bd_get_diskstat );
    if (0 == diskstat) {
        log_error("arg:diskstat is NULL");
        return -1;
    }

    list<bd_disk_usage> disk;

    if (0 != bd_get_diskUsage(&disk)) {
        log_error("get disk usage fail");
        return -1;
    }

    for (list<bd_disk_usage>::iterator it = disk.begin();
         it != disk.end(); ++it) {
        if (!diskstat->empty()) {
            *diskstat += ";";
        }

        *diskstat += it->fsDir + "," + bd_ulonglong2str(it->fsTotal) + "," + bd_ulonglong2str(it->fsTotal - it->fsFree);
    }

    return 0;
}

int bd_get_diskUsage(list<bd_disk_usage>* diskUsage) {
    //BD_TRACE( bd_sysstat, bd_get_diskUsage );

#ifdef WIN32
    DWORD drives = GetLogicalDrives();

    for (int i = 0; i < 25; ++i) {
        //A->Z
        if (drives & (0x0001 << i)) { // drives,从右至左,每一位对应A..Z
            char fsDir[16] = {0};
            ACE_OS::snprintf(fsDir, sizeof(fsDir), "%c:\\", 'A' + i);

            /*DRIVE_UNKNOWN     = 0; {未知}
            DRIVE_NO_ROOT_DIR = 1; {可移动磁盘}
            DRIVE_REMOVABLE   = 2; {软盘}
            DRIVE_FIXED       = 3; {本地硬盘}
            DRIVE_REMOTE      = 4; {网络磁盘}
            DRIVE_CDROM       = 5; {CD-ROM}
            DRIVE_RAMDISK     = 6; {RAM 磁盘}*/
            if (DRIVE_FIXED != GetDriveTypeA(fsDir)) {
                continue;
            }

            ULARGE_INTEGER spaceAvail, spaceTotal, spaceFree;

            if (!GetDiskFreeSpaceExA(fsDir, &spaceAvail, &spaceTotal, &spaceFree)) {
                continue;
            }

            char fsName[256] = {0};

            if (!GetVolumeInformationA(fsDir, fsName, 256, NULL, NULL, NULL, NULL, 0)) {
                continue;
            }

            bd_disk_usage fsUsage;
            fsUsage.fsDir = fsDir;
            fsUsage.fsName = fsName;
            fsUsage.fsTotal = spaceTotal.QuadPart / 1024;
            fsUsage.fsFree = spaceFree.QuadPart / 1024;

            if (0 == fsUsage.fsTotal) {
                continue;
            }

            fsUsage.fsUsage = 100 - (int)(fsUsage.fsFree * 100 / fsUsage.fsTotal);
            diskUsage->push_back(fsUsage);
        }
    }

#else
    FILE* fp = fopen("/etc/mtab", "r");

    if (0 == fp) {
        log_error("open /etc/mtab fail");
        return -1;
    }

    while (1) {
        struct mntent* mt = getmntent(fp);

        if (0 == mt) {
            break;
        }

        bd_disk_usage fsUsage;
        fsUsage.fsName = mt->mnt_fsname;
        fsUsage.fsDir = mt->mnt_dir;

        if (string::npos == fsUsage.fsName.find("/dev/", 0, 5)) {
            continue;
        }

        struct statfs fs;

        if (0 != statfs(fsUsage.fsDir.c_str(), &fs)) {
            continue;
        }

        fsUsage.fsTotal = fs.f_bsize / 1024 * fs.f_blocks;
        fsUsage.fsFree = fs.f_bsize / 1024 * fs.f_bfree;

        if (0 == fsUsage.fsTotal) {
            continue;
        }

        fsUsage.fsUsage = 100 - fsUsage.fsFree * 100 / fsUsage.fsTotal;
        diskUsage->push_back(fsUsage);
    }

    fclose(fp);
#endif

    if (diskUsage->empty()) {
        return -1;
    }

    return 0;
}

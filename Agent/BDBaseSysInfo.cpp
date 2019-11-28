
#include "BDBaseSysInfo.h"
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <math.h>

using namespace std;


CBDBaseSysInfo CBDBaseSysInfo::m_BaseSysInfo;

CBDBaseSysInfo::CBDBaseSysInfo() {

}

CBDBaseSysInfo::~CBDBaseSysInfo() {

}
//获取内存占用
int CBDBaseSysInfo::getHostMemInfo(struct sysinfo &tag_meminfo) {
	memset(&tag_meminfo, 0, sizeof(tag_meminfo));
	return sysinfo(&tag_meminfo);
}

//获取cpu信息
int CBDBaseSysInfo::getProcCpu(CPU_OCCUPY &cpuinfo) {
    
	ifstream inf("/proc/stat");
	if( !inf ) return -1;  // openfile failed

	string strOneLine;
	memset(&cpuinfo, 0, sizeof(cpuinfo));
	getline(inf, strOneLine); // first line
    if(!strOneLine.empty()) {
        sscanf(strOneLine.c_str(),"%s%lu%lu%lu%lu%lu%lu%lu",cpuinfo.name,
			                                   &cpuinfo.user,
			                                   &cpuinfo.nice,
			                                   &cpuinfo.system,
                                               &cpuinfo.idle,
                                               &cpuinfo.iowait,
                                               &cpuinfo.irq,
                                               &cpuinfo.softirq);
    }
	return 0;
}

//获取cpu占用
int CBDBaseSysInfo::calCpuOccupy(const CPU_OCCUPY &cpuinfo_fir, const CPU_OCCUPY &cpuinfo_sec) {

    unsigned long old_cputime, new_cputime ;	 
    unsigned long idle_diff, total_diff;
    int cpu_use = 0;   

    old_cputime = (unsigned long) (cpuinfo_fir.user + cpuinfo_fir.nice + cpuinfo_fir.system +cpuinfo_fir.idle+cpuinfo_fir.iowait+cpuinfo_fir.irq+cpuinfo_fir.softirq);
    new_cputime = (unsigned long) (cpuinfo_sec.user + cpuinfo_sec.nice + cpuinfo_sec.system +cpuinfo_sec.idle+cpuinfo_sec.iowait+cpuinfo_sec.irq+cpuinfo_sec.softirq);
    total_diff  = new_cputime-old_cputime;
	idle_diff	= (unsigned long) (cpuinfo_sec.idle	- cpuinfo_fir.idle);	   

	if(total_diff != 0)
    cpu_use = (int)ceil((double)(total_diff-idle_diff)*100/(total_diff));
	else cpu_use = 0;

	return cpu_use;

}

//获取硬盘占用
int CBDBaseSysInfo::getDiskOccupy()
{
    int maxDiskOccupy = 0;

    FILE *procpt;
    procpt = popen("df -hl", "r");

    char line[100] = {0};
    while (fgets(line, sizeof(line), procpt))
    {
        string strLine(line);
        if(strLine.find("/dev") != string::npos)
        {
            string::size_type idx = strLine.find("%");
            if(idx == string::npos) continue;

            string::size_type firstPos = idx - 1;
            while(strLine[firstPos] >= '0' && strLine[firstPos] <= '9')
                firstPos--;

            firstPos += 1;
            string strDiskUse = strLine.substr(firstPos, idx);
            int nDiskUse = atoi(strDiskUse.c_str());
            if(maxDiskOccupy < nDiskUse) maxDiskOccupy = nDiskUse;
        }
    }
    pclose(procpt);
    return maxDiskOccupy;
}





















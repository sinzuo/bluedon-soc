
#include "bd_ping.h"
#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"
#include "ace/OS_NS_sys_time.h"
#include "ace/OS_NS_unistd.h"
#include <iostream>
using namespace std;

bd_ping::bd_ping()
{
}

bd_ping::~bd_ping()
{
}

bool bd_ping::ping(const std::string& ip, int timout, int* timecost)
{    
    // 发送的测试数据
    ACE_INET_Addr addrRemote;
    int nErr = addrRemote.set("0", ip.c_str());
    if (nErr != 0)
    {
        return false;
    }
    nErr = m_ping_sock.open(ACE_INET_Addr::sap_any);
    if (nErr != 0)
    {
        return false;
    }
    // 用于计算对端回应时间
    ACE_Time_Value tvTimeBegin;
    ACE_Time_Value tvTimeEnd;
    ACE_Time_Value tvRecvCost;

    ACE_Time_Value tvTimeout(0, timout);
    tvTimeBegin = ACE_OS::gettimeofday();
    //int nmcData = m_ping_sock.make_echo_check(addrRemote, true, &tvTimeout);
    //0表示不连接
    int nmcData = m_ping_sock.make_echo_check(addrRemote, 0, &tvTimeout);
	if (nmcData != 0)
    {
        m_ping_sock.close();
		m_ping_sock.dump();
		
        return false;
    }
    tvTimeEnd = ACE_OS::gettimeofday();

    tvRecvCost = tvTimeEnd - tvTimeBegin;
    *timecost = (int) tvRecvCost.usec();  //转换为微秒数
    //tvRecvCost.msec();  // 转换为毫秒数

    m_ping_sock.close();
	m_ping_sock.dump();
    return true;
}

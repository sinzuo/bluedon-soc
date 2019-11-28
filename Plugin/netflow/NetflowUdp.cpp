
#include "NetflowUdp.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils/bd_common.h"
#include <fcntl.h>

using namespace std;


ACE_Reactor* netUdpClientService::pReactor = NULL;
netUdpClientService* netUdpClientService::m_udpInstance = NULL;

ACE_SOCK_Dgram& netUdpClientService::peer(void)
{
	return sock_;
}

int netUdpClientService::open(int port)
{
        //pReactor = new ACE_Reactor( (ACE_Reactor_Impl *)new ACE_TP_Reactor(), true);
	reactor(pReactor);
	ACE_INET_Addr port_to_listen(port);
	sock_.get_local_addr(port_to_listen);
	sock_.open(port_to_listen);
    fcntl(sock_.get_handle(), F_SETFD, FD_CLOEXEC);
	int tmp1(1);
    // 开启地址复用选项
	sock_.set_option(SOL_SOCKET, SO_REUSEADDR, &tmp1, sizeof(tmp1));
	//注册读就绪回调函数
	return pReactor->register_handler(this, ACE_Event_Handler::READ_MASK);
}

ACE_HANDLE netUdpClientService::get_handle(void) const
{
	return sock_.get_handle();
}

int netUdpClientService::handle_input(ACE_HANDLE fd)
{
    ACE_INET_Addr remoteAddr;
     memset(m_buf, 0, sizeof(m_buf));
    int rev = peer().recv(m_buf, sizeof(m_buf),remoteAddr);
    if (rev < 72) {  //netflow数据包最少24+48
        return 0;
    }
    if ((rev-24)%48 != 0)  //netflow 包头24个字节，信息流48个字节
    {
         return 0;
    }
    /**/
    netflow_head head_data;
    //添加收集数据ip地址
    const char *agent_ip = remoteAddr.get_host_addr();

    char headstr[24] = {0};
    memcpy(headstr, m_buf, 24);

   // memcpy(head_data.version, headstr, 2);
   // u_short version = ntohs(*((u_short*)(head_data.version)));
    memcpy(head_data.count, headstr+2, 2);
    u_short count = ntohs(*((u_short*)(head_data.count)));
    //memcpy(head_data.SysUptime, headstr+4, 4);
    //u_long SysUptime = ntohl(*((u_long*)(head_data.SysUptime)));
   // NETFLOW_DEBUG_V("SysUptime :"+bd_int2str(SysUptime));
    memcpy(head_data.unix_secs, headstr+8, 4);
    u_long unix_secs = ntohl(*((u_long*)(head_data.unix_secs)));
    //NETFLOW_DEBUG_V("unix_secs :"+bd_int2str(unix_secs));
    //memcpy(head_data.unix_nsecs, headstr+12, 4);
    //u_long unix_nsecs = ntohl(*((u_long*)(head_data.unix_nsecs)));
   //NETFLOW_DEBUG_V("unix_nsecs :"+bd_int2str(unix_nsecs));
   // memcpy(head_data.flow_sequence, headstr+16, 4);
    //u_long flow_sequence = ntohl((u_long)head_data.flow_sequence);
    //NETFLOW_DEBUG_V("flow_sequence :"+bd_int2str(flow_sequence));
   // memcpy(head_data.engine_type, headstr+20, 1);
    //u_short engine_type = *((u_short *)head_data.engine_type)& 0x00FF;
    //memcpy(head_data.engine_id, headstr+21, 1);
    //u_short engine_id = *((u_short *)head_data.engine_id)& 0x00FF;

    char recordstr[48] = {0};
    char temp[1024] = {0};

    if(rev != 24+(48*count)) //判断收到的报文长度是否正确
    {
        return 0;
    }

    for(int i=0; i<count; i++)
    {

        memcpy(recordstr, m_buf+24+(48*i), 48);
        netflow_record record_data;
        netflow_record_result record_result;

        memcpy(record_data.srcaddr, recordstr, 4);  //src ip
        u_short ip1 = (u_short)(record_data.srcaddr[0]&0x00FF);
        u_short ip2 = (u_short)(record_data.srcaddr[1]&0x00FF);
        u_short ip3 = (u_short)(record_data.srcaddr[2]&0x00FF);
        u_short ip4 = (u_short)(record_data.srcaddr[3]&0x00FF);
        sprintf(temp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
        record_result.srcaddr = string(temp);
        //NETFLOW_DEBUG_V("src ip :"+string(temp));
        memset(temp, 0, sizeof(temp));
        ip1 = ip2 = ip3 = ip4 = 0;
        memcpy(record_data.dstaddr, recordstr+4, 4); //dst ip
        ip1 = (u_short)(record_data.dstaddr[0]&0x00FF);
        ip2 = (u_short)(record_data.dstaddr[1]&0x00FF);
        ip3 = (u_short)(record_data.dstaddr[2]&0x00FF);
        ip4 = (u_short)(record_data.dstaddr[3]&0x00FF);
        sprintf(temp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
        record_result.dstaddr = string(temp);
        //NETFLOW_DEBUG_V("dst ip :"+string(temp));
        memset(temp, 0, sizeof(temp));
        memcpy(record_data.nexthop, recordstr+8, 4); //nexthop ip
        ip1 = (u_short)(record_data.nexthop[0]&0x00FF);
        ip2 = (u_short)(record_data.nexthop[1]&0x00FF);
        ip3 = (u_short)(record_data.nexthop[2]&0x00FF);
        ip4 = (u_short)(record_data.nexthop[3]&0x00FF);
        sprintf(temp, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
        record_result.nexthop = string(temp);
        memset(temp, 0, sizeof(temp));

        memcpy(record_data.input, recordstr+12, 2); //eth in
        u_short eth_in = ntohs(*((u_short *)record_data.input));
        record_result.input = eth_in;

        memcpy(record_data.output, recordstr+14, 2); //eth out
        u_short eth_out = ntohs(*((u_short *)record_data.output));
        record_result.output = eth_out;

        memcpy(record_data.dPkts, recordstr+16, 4); //信息流中的数据包
         u_long dPkts = ntohl(*((u_long*)(record_data.dPkts)));
        record_result.dPkts = dPkts;

        memcpy(record_data.dOctets, recordstr+20, 4); //信息流中的字节的总个数
         u_long dOctets = ntohl(*((u_long*)(record_data.dOctets)));
        record_result.dOctets = dOctets;

        memcpy(record_data.First, recordstr+24, 4); //信息流开始时间, 从机器开机开始算
         u_long First = ntohl(*((u_long*)(record_data.First)));       

        memcpy(record_data.Last, recordstr+28, 4); //信息流结束时间
         u_long Last = ntohl(*((u_long*)(record_data.Last)));

        //设置开始时间为record_time+(信息流开始时间-信息流结束时间)
        record_result.First = unix_secs + First - Last;
        //设置结束时间为record_time
        record_result.Last = unix_secs;

        memcpy(record_data.srcport, recordstr+32, 2); // 源设备端口号
         u_short srcport = ntohs(*((u_short*)(record_data.srcport)));
        record_result.srcport = srcport;

        memcpy(record_data.dstport, recordstr+34, 2); // 目标设备端口号
         u_short dstport = ntohs(*((u_short*)(record_data.dstport)));
        record_result.dstport = dstport;

        memcpy(record_data.prot, recordstr+38, 1); // 协议类型
         u_short prot = *((u_short *)record_data.prot)& 0x00FF;
        record_result.prot = prot;

        memcpy(record_data.src_as, recordstr+40, 2); // 源设备as
         u_short src_as = ntohs(*((u_short*)(record_data.src_as)));
        record_result.src_as = src_as;

        memcpy(record_data.dst_as, recordstr+42, 2); // 目标设备as
         u_short dst_as = ntohs(*((u_short*)(record_data.dst_as)));
        record_result.dst_as = dst_as;

        memcpy(record_data.src_mask, recordstr+44, 1); // mask
         u_short src_mask = *((u_short *)record_data.src_mask)& 0x00FF;
        record_result.src_mask = src_mask;

        memcpy(record_data.dst_mask, recordstr+45, 1); // mask
         u_short dst_mask = *((u_short *)record_data.dst_mask)& 0x00FF;
        record_result.dst_mask = dst_mask;

        record_result.flownum = count;
        record_result.record_time = unix_secs;
        record_result.agent_ip = agent_ip;

        //NETFLOW_DEBUG_V(record_result.srcaddr+"~"+record_result.dstaddr+"~"+record_result.nexthop+"~"+bd_int2str(record_result.input));

        m_inputMutex.lock();
        if(m_netflow_result.size() < m_netflow_result.max_size())
        {
           m_netflow_result.push_back(record_result);
        }
        m_inputMutex.unlock();
    }


    return 0;
}

// 释放相应资源
int netUdpClientService::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
    if (mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    mask = ACE_Event_Handler::ALL_EVENTS_MASK |
    ACE_Event_Handler::DONT_CALL;
    pReactor->remove_handler(this, mask);
    sock_.close();
    //delete this;    //socket出错时，将自动删除该客户端，释放相应资源
    return 0;
}



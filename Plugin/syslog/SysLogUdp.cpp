
#include "SysLogUdp.h"
#include "SysLogMgr.h"
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>


using namespace std;


UdpClientService* UdpClientService::m_udpInstance = NULL;
//获取sock
ACE_SOCK_Dgram& UdpClientService::peer(void) 
{
	return sock_;
}
//打开监听端口
int UdpClientService::open(int port)
{
	ACE_INET_Addr port_to_listen(port);
	sock_.get_local_addr(port_to_listen);
	sock_.open(port_to_listen);
	//关闭子进程读取父进程文件描述符
	fcntl(sock_.get_handle(), F_SETFD, FD_CLOEXEC);
    int tmp1(1);

	// add by weijian 2016.02.18 开启地址复用选项
    sock_.set_option(SOL_SOCKET, SO_REUSEADDR, &tmp1, sizeof(tmp1));
	//注册读就绪回调函数
	return reactor()->register_handler(this, ACE_Event_Handler::READ_MASK);
}
//获取通讯句柄
ACE_HANDLE UdpClientService::get_handle(void) const 
{
	return sock_.get_handle();
}
//反应器处理函数
int UdpClientService::handle_input(ACE_HANDLE fd)
{
    ACE_INET_Addr remoteAddr;
    if(sizeof(m_buf) == buflen)  //避免爆缓存
    {
        memset(m_buf, 0, sizeof(m_buf));
        buflen = 0;
    }
    int rev = peer().recv(m_buf+buflen, sizeof(m_buf)-buflen,remoteAddr);
    if (rev < 0) {
        return -1;
    }
	/*
    buflen += rev;
    char* msgaddr = strchr(m_buf, '\n');
	string strTmp;
    for(; msgaddr != NULL; msgaddr = strchr(m_buf ,'\n')) {		
    	char tempmsg[10240]={0};
    	memmove(tempmsg, m_buf, msgaddr-m_buf);
        //SYSLOG_INFO_V("recvmsg:"+string(tempmsg));
        ShareData::m_inputMutex.lock();
		strTmp = tempmsg;		
        ShareData::m_strLogList.push_back(strTmp);
        ShareData::m_inputMutex.unlock();
	    buflen = buflen - (msgaddr-m_buf+1);
	    memmove(m_buf, msgaddr+1, buflen);
	    memset(m_buf+buflen, 0, sizeof(m_buf)-buflen);
    }
	*/
	
	//根据分隔符来分割buff
	buflen += rev;
    string  str_buf = m_buf;
    size_t newPos = str_buf.find(ShareData::m_strDelimiter);
    while(newPos != string::npos)
    {
        string smg = str_buf.substr(0, newPos);
        ShareData::m_inputMutex.lock();
		if(ShareData::list_num < ShareData::m_strLogList.max_size()/10)
		{
			ShareData::m_strLogList.push_back(smg);
			ShareData::list_num++;
		}
        ShareData::m_inputMutex.unlock();

        str_buf = str_buf.substr(newPos + ShareData::m_strDelimiter.size(), str_buf.size());
        newPos = str_buf.find(ShareData::m_strDelimiter);
    }
    memset(m_buf, 0, sizeof(m_buf));
    sprintf(m_buf,"%s",str_buf.c_str());
    buflen = str_buf.size();
    return 0;
}

// 释放相应资源
int UdpClientService::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
    if (mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    mask = ACE_Event_Handler::ALL_EVENTS_MASK |
    ACE_Event_Handler::DONT_CALL;
    reactor()->remove_handler(this, mask);
    sock_.close();
    //delete this;    // 由外部调用被动释放
    return 0;
}



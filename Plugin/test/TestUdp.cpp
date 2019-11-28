
#include "TestUdp.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;


ACE_Reactor* TestUdpClientService::pReactor = NULL;
TestUdpClientService* TestUdpClientService::m_udpInstance = NULL;

ACE_SOCK_Dgram& TestUdpClientService::peer(void) 
{
	return sock_;
}

int TestUdpClientService::open(int port)
{
        //pReactor = new ACE_Reactor( (ACE_Reactor_Impl *)new ACE_TP_Reactor(), true);
	reactor(pReactor);
    
	ACE_INET_Addr port_to_listen(port);
	sock_.get_local_addr(port_to_listen);
	sock_.open(port_to_listen);
	int tmp1(1);

	// add by weijian 2016.02.18 开启地址复用选项
	sock_.set_option(SOL_SOCKET, SO_REUSEADDR, &tmp1, sizeof(tmp1));
	//注册读就绪回调函数
	return pReactor->register_handler(this, ACE_Event_Handler::READ_MASK);
}

ACE_HANDLE TestUdpClientService::get_handle(void) const 
{
	return sock_.get_handle();
}

int TestUdpClientService::handle_input(ACE_HANDLE fd)
{
    ACE_INET_Addr remoteAddr;
    int rev = peer().recv(m_buf+buflen, sizeof(m_buf)-buflen,remoteAddr);
	if (rev <= 0) {
        return -1;
    }
    buflen += rev;
    char* msgaddr = strchr(m_buf, '\n');
    for(; msgaddr != NULL; msgaddr = strchr(m_buf ,'\n')) {		
    	char tempmsg[10240]={0};
    	memmove(tempmsg, m_buf, msgaddr-m_buf);
		string str = tempmsg;
		TEST_DEBUG_V(tempmsg);
		m_inputMutex.lock();
		string strTmp = tempmsg;		
		m_strLogList.push_back(strTmp);
		m_inputMutex.unlock();		
	    buflen = buflen - (msgaddr-m_buf+1);
	    memmove(m_buf, msgaddr+1, buflen);
	    memset(m_buf+buflen, 0, sizeof(m_buf)-buflen);
    }
    return 0;
}

// 释放相应资源
int TestUdpClientService::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
    if (mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    mask = ACE_Event_Handler::ALL_EVENTS_MASK |
    ACE_Event_Handler::DONT_CALL;
    pReactor->remove_handler(this, mask);
    sock_.close();
	if( m_udpInstance != NULL ) {
		delete m_udpInstance;
		m_udpInstance = NULL;
	}
    //delete this;    //socket出错时，将自动删除该客户端，释放相应资源
    return 0;
}



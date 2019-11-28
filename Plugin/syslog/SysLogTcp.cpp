
#include "SysLogTcp.h"
#include "SysLogMgr.h"
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;


ACE_SOCK_Stream& TcpClientService::peer(void) 
{
	return sock_;
}

int TcpClientService::open(void)
{	
	//注册读就绪回调函数
	return reactor()->register_handler(this, ACE_Event_Handler::READ_MASK);
}

ACE_HANDLE TcpClientService::get_handle(void) const 
{
	return sock_.get_handle();
}

int TcpClientService::handle_input(ACE_HANDLE fd)
{
    if(sizeof(m_buf) == buflen)  //避免爆缓存
    {
        memset(m_buf, 0, sizeof(m_buf));
        buflen = 0;
    }
    int rev = peer().recv(m_buf+buflen,sizeof(m_buf)-buflen);
	if (rev <= 0) {
        return -1;
    }
	/*
    buflen += rev;
    char* msgaddr = strchr(m_buf, '\n');
	string strTmp;
    for(; msgaddr != NULL; msgaddr = strchr(m_buf ,'\n')) {		
    	char tempmsg[10240]={0};
    	memmove(tempmsg, m_buf, msgaddr-m_buf);
		//SYSLOG_DEBUG_V(string(tempmsg));
        ShareData::m_inputMutex.lock();
		strTmp = tempmsg;		
        ShareData::m_strLogList.push_back(strTmp);
        ShareData::m_inputMutex.unlock();
	    buflen = buflen - (msgaddr-m_buf+1);
	    memmove(m_buf, msgaddr+1, buflen);
	    memset(m_buf+buflen, 0, sizeof(m_buf)-buflen);
    }*/
	buflen += rev;
    string  str_buf = m_buf;
    size_t newPos = str_buf.find(ShareData::m_strDelimiter);
    while(newPos != string::npos)
    {
        string smg = str_buf.substr(0, newPos);
        ShareData::m_inputMutex.lock();
        //if(ShareData::m_strLogList.size() < ShareData::m_strLogList.max_size()/10)
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
int TcpClientService::handle_close(ACE_HANDLE, ACE_Reactor_Mask mask)
{
    if (mask == ACE_Event_Handler::WRITE_MASK)
        return 0;
    mask = ACE_Event_Handler::ALL_EVENTS_MASK |
    ACE_Event_Handler::DONT_CALL;
    reactor()->remove_handler(this, mask);
    sock_.close();
    //delete this;    // 由外部调用被动释放
    return 0;
}//*/


/**************Acceptor Class**************/
TcpClientAcceptor* TcpClientAcceptor::m_tcpInstance = NULL;

TcpClientAcceptor::~TcpClientAcceptor()
{
    this->handle_close(ACE_INVALID_HANDLE, 0);
}

int TcpClientAcceptor::open(int port) 
{ 
    ACE_INET_Addr listen_addr(port);
    acceptor_.get_local_addr(listen_addr);
    //acceptor_.open(listen_addr);        
    if (this->acceptor_.open(listen_addr, 1) == -1) 
    { 
        printf("open port fail...\n"); 
        return -1; 
    } 
    
    fcntl(acceptor_.get_handle(), F_SETFD, FD_CLOEXEC); //设置描述符在子进程中关闭

    int tmp1(1);
    //开启地址复用选项
    acceptor_.set_option(SOL_SOCKET, SO_REUSEADDR, &tmp1, sizeof(tmp1));
    
    //注册接受连接回调事件 
    return this->reactor()->register_handler(this, ACE_Event_Handler::ACCEPT_MASK); 
}

ACE_HANDLE TcpClientAcceptor::get_handle(void) const 
{ 
    return this->acceptor_.get_handle(); 
}

int TcpClientAcceptor::handle_input( ACE_HANDLE fd ) 
{ 
    TcpClientService *client = new TcpClientService();
    auto_ptr<TcpClientService> p(client);
    if (this->acceptor_.accept(client->peer()) == -1)
    {
        printf("accept client fail...\n");
        return -1;
    }
    p.release();
    client->reactor(this->reactor());
    if (client->open() == -1)
        client->handle_close(ACE_INVALID_HANDLE, 0);
    
    return 0; 
}

int TcpClientAcceptor::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask) 
{ 
    if (this->acceptor_.get_handle() != ACE_INVALID_HANDLE) 
    { 
        ACE_Reactor_Mask m = ACE_Event_Handler::ACCEPT_MASK | ACE_Event_Handler::DONT_CALL; 
        this->reactor()->remove_handler(this, m); 
        this->acceptor_.close(); 
    } 
    
    return 0; 
}  

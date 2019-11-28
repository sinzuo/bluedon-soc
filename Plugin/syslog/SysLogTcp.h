/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SysLogTcp.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SYSLOG_TCP_H_
#define _SYSLOG_TCP_H_

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
#include "ace/Reactor.h"
#include "ace/OS.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/SOCK_Connector.h"
#include "ace/Auto_Ptr.h"
#include "ace/Message_Block.h"


class TcpClientService : public ACE_Event_Handler
{
public: 
	TcpClientService(): buflen(0) {
		memset(m_buf, 0, sizeof(m_buf));		
	}
	//TcpClientService(const TcpClientService &);

public:
    ACE_SOCK_Stream& peer(void);
    int open(void);	//new modified
    virtual ACE_HANDLE get_handle(void) const; 
    virtual int handle_input(ACE_HANDLE fd);
    virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask mask);

protected:
    ACE_SOCK_Stream sock_;
    char m_buf[10240];
    int buflen; 
	
};//*/


/**************Acceptor Class**************/
class TcpClientAcceptor : public ACE_Event_Handler 
{ 
private:
    TcpClientAcceptor(){}  //需要加{}进行定义, 否则运行时会出现找不到符号错误
    TcpClientAcceptor(const TcpClientAcceptor &);
    
public: 
    virtual ~TcpClientAcceptor(); 
    int open(int port);    
    virtual ACE_HANDLE get_handle(void) const;
    virtual int handle_input(ACE_HANDLE fd);
    virtual int handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask);
       
    static TcpClientAcceptor*  inistance()  //singleton
    {
		if ( m_tcpInstance == NULL ) 
			m_tcpInstance = new TcpClientAcceptor();
		
		return m_tcpInstance;
    }
	static void Destroy() 
    {
		if( m_tcpInstance != NULL ) 
        {
			delete m_tcpInstance;
			m_tcpInstance = NULL;
		}
	}
   
protected: 
	static TcpClientAcceptor* m_tcpInstance;
    ACE_SOCK_Acceptor acceptor_; 




}; 


#endif //_SYSLOG_TCP_H_



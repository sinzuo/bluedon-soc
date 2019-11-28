/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SysLogUdp.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SYSLOG_UDP_H_
#define _SYSLOG_UDP_H_

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
#include "ace/Reactor.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/OS.h"
#include "ace/SOCK_Dgram_Mcast.h"
#include "ace/Message_Block.h"


class UdpClientService : public ACE_Event_Handler
{
private: //singleton
	UdpClientService(): buflen(0) {
		memset(m_buf, 0, sizeof(m_buf));
	}
	UdpClientService(const UdpClientService &);

public:
    ACE_SOCK_Dgram& peer(void);  //获取sock
    int open(int port);	         //打开监听端口
    virtual ACE_HANDLE get_handle(void) const;   //获取句柄
    virtual int handle_input(ACE_HANDLE fd);     //反应器处理函数
    virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask mask); // 释放相应资源
    //单例模式
    static UdpClientService*  inistance() {
		if ( m_udpInstance == NULL ) {
			m_udpInstance = new UdpClientService();
		}
		return m_udpInstance;
    }
	//释放资源
	static void Destroy() {
		if( m_udpInstance != NULL ) {
			delete m_udpInstance;
			m_udpInstance = NULL;
		}
	}
   
protected:
	static UdpClientService* m_udpInstance;  //单例指针
    ACE_SOCK_Dgram sock_;   //通讯sock
    char m_buf[10240];    
    int buflen;
};

#endif //_SYSLOG_UDP_H_



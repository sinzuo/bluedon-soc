/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    FlowUdp.h
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
#include "ace/TP_Reactor.h"
#include "Poco/Mutex.h"
#include <list>
#include <string>


#define  FLOW_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("FLOW"), str);
#define  FLOW_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("FLOW"), str);
#define  FLOW_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("FLOW"), str);
#define  FLOW_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("FLOW"), str);

#define  FLOW_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("FLOW"), #str);
#define  FLOW_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("FLOW"), #str);
#define  FLOW_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("FLOW"), #str);
#define  FLOW_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("FLOW"), #str);

//using namespace log4cxx;

class FlowUdpClientService : public ACE_Event_Handler
{
private: //singleton
	FlowUdpClientService(): buflen(0) {
		memset(m_buf, 0, sizeof(m_buf));
		pReactor = NULL;
	}
	FlowUdpClientService(const FlowUdpClientService &);

public:
    ACE_SOCK_Dgram& peer(void);
    int open(int port);	
    virtual ACE_HANDLE get_handle(void) const; 
    virtual int handle_input(ACE_HANDLE fd);
    virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask mask);

    static FlowUdpClientService*  inistance() {
		if ( m_udpInstance == NULL ) {
			m_udpInstance = new FlowUdpClientService();
		}
		return m_udpInstance;
    }
	static void Destroy() {
		if( m_udpInstance != NULL ) {
			delete m_udpInstance;
			m_udpInstance = NULL;
		}
	}
   
protected:
	static FlowUdpClientService* m_udpInstance;
    ACE_SOCK_Dgram sock_;
    char m_buf[10240];
    int buflen;
public:

	Poco::Mutex m_inputMutex;
	std::list<std::string> m_strLogList;
	static ACE_Reactor* pReactor;

};

#endif //_FLOW_UDP_H_



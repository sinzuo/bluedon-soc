/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    NetflowUdp.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _NETFLOW_UDP_H_
#define _NETFLOW_UDP_H_

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
using namespace std;

#define  NETFLOW_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("NETFLOW"), str);
#define  NETFLOW_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("NETFLOW"), str);
#define  NETFLOW_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("NETFLOW"), str);
#define  NETFLOW_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("NETFLOW"), str);

#define  NETFLOW_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("NETFLOW"), #str);
#define  NETFLOW_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("NETFLOW"), #str);
#define  NETFLOW_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("NETFLOW"), #str);
#define  NETFLOW_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("NETFLOW"), #str);

//using namespace log4cxx;

typedef struct _netflow_head{  //netflow 结构体
    char version[2];
    char count[2];
    char SysUptime[4];
    char unix_secs[4];
    char unix_nsecs[4];
    char flow_sequence[4];
    char engine_type[1];
    char engine_id[1];
    char reserved[2];
}netflow_head;

typedef struct _netflow_record
{
    char srcaddr[4];
    char dstaddr[4];
    char nexthop[4];
    char input[2];
    char output[2];
    char dPkts[4];
    char dOctets[4];
    char First[4];
    char Last[4];
    char srcport[2];
    char dstport[2];
    char pad1[1];
    char tcp_flags[1];
    char prot[1];
    char tos[1];
    char src_as[2];
    char dst_as[2];
    char src_mask[1];
    char dst_mask[1];
    char pad2[2];
}netflow_record;

typedef struct _netflow_record_result
{
    string agent_ip;
    string srcaddr;
    string dstaddr;
    string nexthop;
    int input;
    int output;
    u_long dPkts;
    unsigned int dOctets;
    int First;
    int Last;
    int srcport;
    int dstport;
    int prot;
    int src_as;
    int dst_as;
    int src_mask;
    int dst_mask;
    int  flownum;
    u_long  record_time;

}netflow_record_result;

class netUdpClientService : public ACE_Event_Handler
{
private: //singleton
    netUdpClientService(): buflen(0) {
		memset(m_buf, 0, sizeof(m_buf));
		pReactor = NULL;
	}
    netUdpClientService(const netUdpClientService &);

public:
    ACE_SOCK_Dgram& peer(void);
    int open(int port);	
    virtual ACE_HANDLE get_handle(void) const; 
    virtual int handle_input(ACE_HANDLE fd);
    virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask mask);

    static netUdpClientService*  inistance() {
		if ( m_udpInstance == NULL ) {
            m_udpInstance = new netUdpClientService();
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
    static netUdpClientService* m_udpInstance;
    ACE_SOCK_Dgram sock_;
    char m_buf[10240];
    int buflen;
public:

	Poco::Mutex m_inputMutex;
	std::list<std::string> m_strLogList;
    std::list<netflow_record_result> m_netflow_result;
	static ACE_Reactor* pReactor;

};

#endif //_NETFLOW_UDP_H_



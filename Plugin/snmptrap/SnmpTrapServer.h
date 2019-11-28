/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    SnmpTrapServer.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _SNMPTRAP_SERVER_H_
#define _SNMPTRAP_SERVER_H_

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/agent/ds_agent.h>
#include <net-snmp/library/fd_event_manager.h>
#include "Poco/Mutex.h"
#include "utils/bdstring.h"
#include <list>

#define  SNMPTRAP_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SNMPTRAP"), str)
#define  SNMPTRAP_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SNMPTRAP"), str)
#define  SNMPTRAP_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SNMPTRAP"), str)
#define  SNMPTRAP_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SNMPTRAP"), str)

#define  SNMPTRAP_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("SNMPTRAP"), #str)
#define  SNMPTRAP_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("SNMPTRAP"), #str)
#define  SNMPTRAP_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("SNMPTRAP"), #str)
#define  SNMPTRAP_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("SNMPTRAP"), #str)



class CSnmpTrapServer
{
private: //singleton
	CSnmpTrapServer();
	CSnmpTrapServer(const CSnmpTrapServer &);

public:

    static CSnmpTrapServer*  inistance() {
		if ( m_snmptrap_ins == NULL ) {
			m_snmptrap_ins = new CSnmpTrapServer();
		}
		return m_snmptrap_ins;
    }
    static void Destroy() {
        if( m_snmptrap_ins != NULL ) {
            delete m_snmptrap_ins;
            m_snmptrap_ins = NULL;
        }
    }

protected:
	static CSnmpTrapServer* m_snmptrap_ins;
public: // callback
	static int snmp_input(int op, netsnmp_session *session, int reqid, netsnmp_pdu *pdu, void *magic);
	static int pre_parse(netsnmp_session * session, netsnmp_transport *transport, 
		void *transport_data, int transport_data_length);
public:
	int Initlize();
	void SetLinstenPort(string &strListenPorts);
	netsnmp_session *snmptrapd_add_session(netsnmp_transport *t);
	int BindAll();
	void snmptrapd_main_loop();
	void snmptrapd_close_sessions(netsnmp_session * sess_list);

	void setRunningFlag(bool bflag);
	int handle_close(); // 释放资源
    list<string> getLogList();
    void LogList_erase(string src);
    void lockMutex();
    void unlockMutex();
private:
	bool m_brunning;
	string m_listen_ports;
	netsnmp_session *m_session_list;
public:
	static Poco::Mutex m_inputMutex;
	static list<string> m_strLogList;

};

#endif //_SNMPTRAP_SERVER_H_



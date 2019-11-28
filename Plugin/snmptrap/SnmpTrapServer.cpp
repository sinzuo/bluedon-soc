
#include "SnmpTrapServer.h"
#include <errno.h>
#include <iostream>
#include <ctime>


CSnmpTrapServer* CSnmpTrapServer::m_snmptrap_ins = NULL;
Poco::Mutex CSnmpTrapServer::m_inputMutex;
list<string> CSnmpTrapServer::m_strLogList;

CSnmpTrapServer::CSnmpTrapServer() {
	m_brunning     = true;
	m_listen_ports = "udp:162";   // default port
	m_session_list = NULL;
    m_strLogList.clear();
}

list<string> CSnmpTrapServer::getLogList()
{
    return m_strLogList;
}
void CSnmpTrapServer::LogList_erase(string src)
{
    list<string>::iterator it = find(m_strLogList.begin(),m_strLogList.end(),src);
    printf("11111111111111");
    m_strLogList.erase(it);
}
void CSnmpTrapServer::lockMutex()
{
    m_inputMutex.lock();
}
void CSnmpTrapServer::unlockMutex()
{
    m_inputMutex.unlock();
}
void CSnmpTrapServer::snmptrapd_close_sessions(netsnmp_session * sess_list)
{
    netsnmp_session *s = NULL, *next = NULL;

    for (s = sess_list; s != NULL; s = next)
	{
        next = s->next;
        snmp_close(s);
    }
}

int CSnmpTrapServer::Initlize() {
	
    init_snmp("snmptrapd");
	//netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OPTIONALCONFIG, "/etc/snmp/snmptrapd.conf");
	if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_QUIT_IMMEDIATELY)) {
		 /*
		  * just starting up to process specific configuration and then
		  * shutting down immediately. 
		  */
         m_brunning = false;
		 return -1;
	}	
	/*
     * if no logging options on command line or in conf files, use syslog
     */
	if (0 == snmp_get_do_logging())
	{
        snmp_enable_syslog_ident("snmptrapd", LOG_DAEMON);
	}

	return 0;
}

void CSnmpTrapServer::SetLinstenPort(string &strListenPorts) {
	m_listen_ports = strListenPorts;
}

int CSnmpTrapServer::snmp_input(int op, netsnmp_session *session, int reqid, netsnmp_pdu *pdu, void *magic)
{
    string strSnmpTrap;
	struct variable_list *vars;	
    for(vars=pdu->variables; vars!=NULL; vars=vars->next_variable)
    {
        char buf[256] = {0}; //256应该够了吧
		snprint_variable(buf, sizeof(buf), vars->name,
										   vars->name_length,
										   vars);
		//buf[sizeof(buf) - 1] = '\0';
        //print_variable(vars->name, vars->name_length, vars);
        strSnmpTrap += string(buf) ;
		if (vars!=NULL) strSnmpTrap+= " "; // 以空格分隔不同变量
	}

    if(strSnmpTrap.length() == 0) return 1; 

	time_t t = time(NULL);
	struct tm now;
	localtime_r(&t, &now);
	char chLogDate[22] = {0};
	//snmptrap结构里好像没有日志产生时间
	sprintf(chLogDate,"%04d-%02d-%02d %02d:%02d:%02d",now.tm_year+1900
													 ,now.tm_mon+1
													 ,now.tm_mday
													 ,now.tm_hour
													 ,now.tm_min
													 ,now.tm_sec);	
    //Source IP: UDP: [172.16.12.188]:50138
	netsnmp_transport *transport = (netsnmp_transport *) magic;	
    char *chSourceIP = transport->f_fmtaddr(transport, pdu->transport_data, pdu->transport_data_length);
	string strSourceIP = chSourceIP;
	string::size_type pos1, pos2;
	pos1 = strSourceIP.find("[");
	pos2 = strSourceIP.find("]");
	if( pos1 != string::npos || pos2 != string::npos ) {
	    strSourceIP = strSourceIP.substr(pos1+1, pos2-pos1-1);
	}
	
	/*按照和syslog输出的格式拼接snmptrap日志
     *yyyy-mm-dd hh:mi:ss~IP~usr~notice~snmptrap~CONTENT
     */
    string strFS = string("~"); 
	//string strSnmpTrap = string(chLogDate) + strSourceIP + strLogPrefix;
	string strLogPrefix = string(chLogDate) + strFS 
	                    + strSourceIP       + strFS 
	                    + "user"            + strFS   //FACILITY
	                    + "notice"          + strFS   //PRIORITY
	                    + "snmptrap"        + strFS;  //PROGRAM

    strSnmpTrap = strLogPrefix + strSnmpTrap;

	m_inputMutex.lock();
	m_strLogList.push_back(strSnmpTrap);
	m_inputMutex.unlock();

	SNMPTRAP_DEBUG_V("receive a trap: " + strSnmpTrap);
    
    return 1;
}

/* do not support TCP Wrappers */
int CSnmpTrapServer::pre_parse(netsnmp_session * session, netsnmp_transport *transport,
          void *transport_data, int transport_data_length) {
	return 1;
}


netsnmp_session* CSnmpTrapServer::snmptrapd_add_session(netsnmp_transport *t) {
    netsnmp_session sess, *session = &sess, *rc = NULL;

    snmp_sess_init(session);
    session->peername = SNMP_DEFAULT_PEERNAME;  /* Original code had NULL here */
    session->version = SNMP_DEFAULT_VERSION;
    session->community_len = SNMP_DEFAULT_COMMUNITY_LEN;
    session->retries = SNMP_DEFAULT_RETRIES;
    session->timeout = SNMP_DEFAULT_TIMEOUT;
    session->callback = snmp_input;
    session->callback_magic = (void *) t;
    session->authenticator = NULL;
    sess.isAuthoritative = SNMP_SESS_UNKNOWNAUTH;

    rc = snmp_add(session, t, pre_parse, NULL);
    if (rc == NULL)
	{
        snmp_sess_perror("snmptrapd", session);
    }
    return rc;
}


int CSnmpTrapServer::BindAll() {
    vector<string> vecListenPorts; 
	string strToken = ",";
	string_split(m_listen_ports,strToken, vecListenPorts);
    vector<string>::iterator iter_tmp = vecListenPorts.begin(); 

	netsnmp_session *session_list = NULL;
	netsnmp_session *session_node = NULL;
	for(;iter_tmp != vecListenPorts.end(); iter_tmp++) {
		netsnmp_transport *transport = NULL;
        transport = netsnmp_transport_open_server("snmptrap", (*iter_tmp).c_str());

        if( transport == NULL ) {
            SNMPTRAP_ERROR_V("netsnmp_transport_open_server failed! Error: " + string(strerror(errno)));
			snmptrapd_close_sessions(session_list);
			SOCK_CLEANUP;      
			return -1;
        } else {
			session_node = snmptrapd_add_session(transport);
			if (session_node == NULL) {
			 /*
			  * Shouldn't happen?  We have already opened the transport
			  * successfully, so what could have gone wrong?  
			  */
			    snmptrapd_close_sessions(session_list);
				netsnmp_transport_free(transport);
				SOCK_CLEANUP;
                SNMPTRAP_ERROR_S(snmptrapd_add_session failed!);
				return -1;
			} else {
				session_node->next = session_list;
				session_list = session_node;
			}
        }
	}
	m_session_list = session_list;
	
	return 0;
}
	
void CSnmpTrapServer::snmptrapd_main_loop() {
	
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	
	while (m_brunning)
	{		
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);

		int numfds = 0;
		int block  = 0;
		struct timeval timeout = {3,0}; // default 3 secs
		struct timeval *tvp = &timeout;
		/*
		 * input:  set to 1 if input timeout value is undefined  
		 * set to 0 if input timeout value is defined	 
		 * output: set to 1 if output timeout value is undefined 
		 * set to 0 if output rimeout vlaue id defined	 
		 */
		snmp_select_info(&numfds, &readfds, tvp, &block);
		//if (block == 1) tvp = NULL;  /* block without timeout */

		netsnmp_external_event_info(&numfds, &readfds, &writefds, &exceptfds);
		int count = select(numfds, &readfds, &writefds, &exceptfds, tvp);

		if (count > 0)
		{
			netsnmp_dispatch_external_events(&count, &readfds, &writefds, &exceptfds);
			/* If there are any more events after external events, then try SNMP events. */
			if (count > 0) {
				snmp_read(&readfds);
			}
		} else {
			switch (count) {
				case 0:
					snmp_timeout();
					break;
				case -1:
					if (errno == EINTR)	continue;
					snmp_log_perror("select");
					m_brunning = false;
					break;
				default:
					fprintf(stderr, "select returned %d\n", count);
					m_brunning = false;
			}
		}
		run_alarms();
	}

	snmptrapd_close_sessions(m_session_list);
    //snmp_shutdown("snmptrapd");  //should't shutdown, otherwise it will restart failed
	snmp_disable_log();
	SOCK_CLEANUP;

}


void CSnmpTrapServer::setRunningFlag(bool bflag) {
    m_brunning = bflag;
}

// 释放相应资源
int CSnmpTrapServer::handle_close() {
	
	if( m_snmptrap_ins != NULL ) {
		delete m_snmptrap_ins;
		m_snmptrap_ins = NULL;
	}
    return 0;
}



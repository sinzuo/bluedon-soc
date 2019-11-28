#ifndef _TEST_UDP_H_
#define _TEST_UDP_H_

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


#define  TEST_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("TEST"), str);
#define  TEST_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("TEST"), str);
#define  TEST_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("TEST"), str);
#define  TEST_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("TEST"), str);

#define  TEST_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("TEST"), #str);
#define  TEST_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("TEST"), #str);
#define  TEST_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("TEST"), #str);
#define  TEST_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("TEST"), #str);

//using namespace log4cxx;

class TestUdpClientService : public ACE_Event_Handler
{
private: //singleton
	TestUdpClientService(): buflen(0) {
		memset(m_buf, 0, sizeof(m_buf));
		pReactor = NULL;
	}
	TestUdpClientService(const TestUdpClientService &);

public:
    ACE_SOCK_Dgram& peer(void);
    int open(int port);	
    virtual ACE_HANDLE get_handle(void) const; 
    virtual int handle_input(ACE_HANDLE fd);
    virtual int handle_close(ACE_HANDLE, ACE_Reactor_Mask mask);

    static TestUdpClientService*  inistance() {
		if ( m_udpInstance == NULL ) {
			m_udpInstance = new TestUdpClientService();
		}
		return m_udpInstance;
    }
   
protected:
	static TestUdpClientService* m_udpInstance;
    ACE_SOCK_Dgram sock_;
    char m_buf[10240];
    int buflen;
public:

	Poco::Mutex m_inputMutex;
	std::list<std::string> m_strLogList;
	static ACE_Reactor* pReactor;

};

#endif //_TEST_UDP_H_



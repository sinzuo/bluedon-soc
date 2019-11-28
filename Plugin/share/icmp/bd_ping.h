

#ifndef __BD_PING_H__
#define	__BD_PING_H__

#include "ace/Ping_Socket.h"
#include <string>

class bd_ping {
public:
    bd_ping();
    ~bd_ping();
    
    bool ping(const std::string& ip, int timout, int* timecost);
    
private:
    bd_ping(const bd_ping& orig);
    bd_ping& operator=(const bd_ping& orig);
    
private:
    ACE_Ping_Socket m_ping_sock;
};

#endif	/* __BD_PING_H__ */


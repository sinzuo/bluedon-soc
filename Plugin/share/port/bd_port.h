#ifndef _BD_PORT_H_
#define _BD_PORT_H_

#include <string>

class CScanPort {
public:
	static bool scanTcpPort(std::string &strIp, int nPort);
	static bool scanUdpPort(std::string &strIp, int nPort);
    static bool checkPort(std::string &strIp, int nPort, int nProtocal = 0 /* 0:tcp 1:udp*/);

private:
};

#endif //_BD_PORT_H_





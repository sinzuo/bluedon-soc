
#include "bd_port.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>

#include <iostream>

using namespace std;

bool CScanPort::scanTcpPort(string &strIp, int nPort) {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 ) {
		perror("socket");
		//cout<<strerror(errno)<<endl;
		return false;
	}
	struct sockaddr_in dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(nPort);
	if( inet_aton(strIp.c_str(), &dest_addr.sin_addr) == 0 ) {// failed
        perror("inet_aton");	
		//cout<<strerror(errno)<<endl;
	}
	socklen_t len = sizeof(dest_addr);
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    int n;
	if( (n = connect(sockfd, (struct sockaddr *)&dest_addr, len)) < 0) {
		if ( errno != EINPROGRESS ) return false;
	} else if( n == 0) {// connect immediaely
        close(sockfd);
		return true;
	}
	
	fd_set rset, wset;
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	wset = rset;
	struct timeval tval;
	tval.tv_sec = 1; tval.tv_usec = 0;	//timeout 默认1秒
	int m = select(sockfd+1, &rset, &wset, NULL, &tval );
	if ( m == 0 ) { //timeout
		close(sockfd);
		return false;
	}	
	/* 可能因服务器的实现不同而存在可移植性问题
	  if ( FD_ISSET(sockfd, &wset) || !FD_ISSET(sockfd, rset)) { 
		close(sockfd);
		return true;
	}*/
	if ( FD_ISSET(sockfd, &wset) || FD_ISSET(sockfd, &rset)) {
		socklen_t len = sizeof(errno);
		getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &errno, &len);
		bool bRet = (errno == 0)?true:false;
		close(sockfd);		
		return bRet;
	}
	//select error
	close(sockfd);
	return false;
}

bool CScanPort::scanUdpPort(string &strIp, int nPort) {

	return false;
}

bool CScanPort::checkPort(string &strIp, int nPort, int nProtocal/*= 0 0:tcp 1:udp*/) {
	if ( nProtocal == 0 ) return scanTcpPort(strIp, nPort);
    else return false;
	return true;
}



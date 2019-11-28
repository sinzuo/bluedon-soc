#ifndef __BD_SSH_H__
#define __BD_SSH_H__
#include <curl/curl.h>
#include <exception>
#include <string>


#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "common/bd_basic.h"
#include "netbase/bd_netbase.h"
#include "libssh2.h"


using namespace std;

namespace netprotocol
{
	class BD_EXPORT_CLASS bd_ssh
	{
	public:
        bd_ssh();
		~bd_ssh();
//		// 执行命令
//		int execute(const std::string& cmdStr);
		//登录
		void login(const std::string& path,const std::string& username=string(""),const std::string& password=string(""));

		// 执行命令
		string command(const string& command) throw(bd_net_exception);


//		//SSH重命名文件（连接已建立）
//		int rename(const std::string & srcfile,const std::string & dstfile);
	protected:
//		string m_serverurl;
        string m_serverhost;
        string m_serverport;
		string m_username;
		string m_password;
		int sock;
        LIBSSH2_SESSION *session;
        LIBSSH2_CHANNEL *channel;
		//char m_userpwd[256];

	};
}

#endif





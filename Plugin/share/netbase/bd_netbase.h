#ifndef __BD_NETBASE_H__
#define __BD_NETBASE_H__
#include "common/bd_basic.h"
#include "exception/bd_net_exception.h"
#include <curl/curl.h>
#include <exception>
#include <stdexcept>
#include <string>
#include <list>
#include "ace/Task.h"

using namespace std;

namespace netprotocol
{
	struct fileinfo
	{
		char filename[256];
		time_t modifytime;
		double length;
	};

	class BD_EXPORT_CLASS bd_netbase
	{
	public:
		bd_netbase() throw(runtime_error);
		virtual ~bd_netbase();

	   	void login(const std::string& path,const std::string& username=string(""),const std::string& password=string(""));

		virtual void downloadfile(const std::string& path,const std::string& savefilename) throw(bd_net_exception);

		virtual void getinfo(const std::string& path,struct fileinfo*);

		virtual void uploadfile(const std::string& path,const std::string& filename) throw(bd_net_exception,std::logic_error);

		virtual void command(const std::string& commandstr,std::string* result) throw(bd_net_exception);
	protected:
		virtual void buildstate();
	protected:
		std::string m_serverurl;
		std::string m_username;
		std::string m_password;
		char m_userpwd[256];
		CURL* m_curl;
		static ACE_Thread_Mutex s_mutex;
	};
}
#endif


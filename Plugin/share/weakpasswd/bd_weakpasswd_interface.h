#ifndef __BD_WEAKPASSWD_INTERFACE__
#define __BD_WEAKPASSWD_INTERFACE__

#include <vector>
#include <string>

using namespace std;

#ifndef __cplusplus
extern "C"{
#endif

	typedef struct weakpassword{
		//应用名称，应用端口，应用软件用户名，应用软件密码
		string servicenm;
		int serviceport;
		string username;
		string pwd;

		weakpassword() : serviceport(0)
		{}
	}WeakpasswdInfo,*pWeakpasswdInfo;

	
	typedef std::vector<WeakpasswdInfo>  WeakpasswdInfoVec;


	/*
	  获取指定ip机器下的服务弱密码列表
	*/
	int GetWeakPassword(const char *neip, WeakpasswdInfoVec &lis);
	

#ifndef __cplusplus
}
#endif  // extern "C"

#endif  //__BD_WEAKPASSWD_INTERFACE__


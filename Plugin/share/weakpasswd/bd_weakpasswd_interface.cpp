#include <cstdio>
#include <vector>
//#include <iostream>
#include <pthread.h>
#include <map>

#include "bd_weakpasswd_interface.h"
#include "weak.h"  //TODO
#include "json/json.h"
//#include "bd_log_macros.h"  //modified in 2017.01.06

using namespace std;

int _parse_result(char *retStr, WeakpasswdInfoVec &vec)
{
//	BD_TRACE(GetWeakPassword, _parse_result);
//	BD_TRACE(_parse_result,"_parse_result run");

	Json::Reader reader;  
	Json::Value root;  
	Json::Value ftproot;  
	Json::Value sshroot;  
	Json::Value rdproot;  
	Json::Value telnetroot;  
	Json::Value mssqlroot;  
	Json::Value mysqlroot;  
	Json::Value smbroot;  
	if(!reader.parse(string(retStr), root))  
	{  
		printf("fail to parse.");  
		return -1;  
	}  

	ftproot = root["ftp"];
	sshroot = root["ssh"];
	rdproot = root["rdp"];
	telnetroot = root["telnet"];
	mssqlroot = root["mssql"];
	mysqlroot = root["mysql"];
	smbroot = root["smb"];

	for(int i=0; i< ftproot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "ftp";
		wpinfo.serviceport = ftproot[key]["port"].asInt();  
		wpinfo.username = ftproot[key]["user"].asString();
		wpinfo.pwd = ftproot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("ftp:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	for(int i=0; i< sshroot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "ssh";
		wpinfo.serviceport = sshroot[key]["port"].asInt();  
		wpinfo.username = sshroot[key]["user"].asString();
		wpinfo.pwd = sshroot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("ssh:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	for(int i=0; i< rdproot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "rdp";
		wpinfo.serviceport = rdproot[key]["port"].asInt();  
		wpinfo.username = rdproot[key]["user"].asString();
		wpinfo.pwd = rdproot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("rdp:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	for(int i=0; i< telnetroot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "telnet";
		wpinfo.serviceport = telnetroot[key]["port"].asInt();  
		wpinfo.username = telnetroot[key]["user"].asString();
		wpinfo.pwd = telnetroot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("telnet:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	for(int i=0; i< mssqlroot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "mssql";
		wpinfo.serviceport = mssqlroot[key]["port"].asInt();  
		wpinfo.username = mssqlroot[key]["user"].asString();
		wpinfo.pwd = mssqlroot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("mssqlroot:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	for(int i=0; i< mysqlroot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "mysql";
		wpinfo.serviceport = mysqlroot[key]["port"].asInt();  
		wpinfo.username = mysqlroot[key]["user"].asString();
		wpinfo.pwd = mysqlroot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("mysql:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	for(int i=0; i< smbroot.size(); ++i)  
	{  
		WeakpasswdInfo wpinfo;

		char key[10] = {0};
		snprintf(key, sizeof(key), "%d", i+1);
		wpinfo.servicenm = "smb";
		wpinfo.serviceport = smbroot[key]["port"].asInt();  
		wpinfo.username = smbroot[key]["user"].asString();
		wpinfo.pwd = smbroot[key]["password"].asString();

		vec.push_back(wpinfo);

		printf("smb:port[%d],user['%s'],password['%s']",wpinfo.serviceport,wpinfo.username.c_str(),wpinfo.pwd.c_str());
	} 

	return 0;

}

/*
void *scan_thread_func(void *argv)
{
	char *ipStr = (char*)argv;
	weak_main(ipStr);
	return NULL;
}
*/

int GetWeakPassword(const char *neip, WeakpasswdInfoVec &vec)
{
	//	BD_TRACE(GetWeakPassword, GetWeakPassword);
	//	BD_TRACE(GetWeakPassword,"GetWeakPassword run");

	//检查输入IP是否合法
	if (neip == NULL)
	{
		printf("neip is null");
		return -1;
	}

	//创建一个扫描空间,传入IP地址
	void *workspace = weak_createworkspace((char *)neip);
	//扫描结束后返回扫描结果,传入空间
	char *retStr = NULL;
	if ( workspace != NULL ) {
		retStr = weak_main(workspace); //返回的rerStr为json格式字符串
	} else {
	    printf("weak_createworkspace failed");
		return -2;
	}
	
	if (retStr != NULL)
	{
		printf("Weakpwd return:[%s]",retStr);
		_parse_result(retStr,vec);  //处理扫描结果
	}
	else
	{
		printf("Can not find weak password!\n");
	}
    //停止扫描工作 @add in 2017.06.12
    weak_stopscand(workspace);
	//释放工作空间,释放空间后不能再读取保存在内存中的结果
	weak_freeworkspace(workspace);
	workspace = NULL;    

	return 0;

}


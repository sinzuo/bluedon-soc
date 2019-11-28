
#ifndef __BD_HTTPS_CONTROL_H__
#define __BD_HTTPS_CONTROL_H__

#include <string>
#include <list>
#include "bd_https.h"

using namespace std;

struct Host
{
	string Ip;
	int Port;
};

struct User
{
	string Name;
	string Password;
};

struct NELogin
{
	Host host;
	string ProtoType;
	User user;
};

struct mac_policy
{
	string target;
	string action;
	string enable;
	string protocol;
	string macaddr;
	string srcip;
	string srcport;
	string desip;
	string desport;
	string packets;
	string connections;
	string stimehour;
	string stimemin;
	string etimehour;
	string etimemin;
	string exclude;
};

class BD_EXPORT_CLASS bd_https_control
{
public:
    
	bd_https_control(const string& serverurl);
	/**
	 * 认证请求
	 * @param username      用户名
	 * @param pwd   		密码
	 */
	void authentication(const string& username,const string& pwd)throw(bd_net_exception,std::runtime_error);
	/**
	 * 重启设备
	 */
	string reboot()throw(bd_net_exception);
	/**
	 * 关闭设备
	 */
	string shutdown()throw(bd_net_exception);
    string factory_recover()throw(bd_net_exception);
	string restart_service(const string& servicename) throw(bd_net_exception);
	string stop_service(const string& servicename) throw(bd_net_exception);
	string syntime(const string& serverip) throw(bd_net_exception);
	void backuppolicy(const string& filetype,
			const string& savefilename)throw(bd_net_exception);
	string uploadpolicy(const string& filetype,
			const string& filename)throw(bd_net_exception,std::logic_error);
	string getstatus(const string& statustype);
	string ipsession(const string& target,const string& action,
			const string& enable,const string& protocol,const string& srcip,
			const string& srcport,const string& desip,const string& desport);
	string macmanage(const struct mac_policy& macpolicy);
	string scannet(const string& beginip,const string& endip) throw(bd_net_exception);
	void set_verify_file(const std::string& certfilename);
	void getNELogin(const string& iplist,list<NELogin*>* nelogins);
	void flowmonitor(const string& filetype,const string& savefilename)throw(bd_net_exception);
	void GetIdsRules();
	void SetIdsRules(std::string &rules);
private:		
	string checkresult(string content,string flags) throw (bd_net_exception);
	int saveresult(string &content, string &filename); 
private:
	string m_serverurl;
	string m_username;
	string m_password;
	string m_sessionkey;
	string m_verify_file;
public:
	string m_result;
};

#endif

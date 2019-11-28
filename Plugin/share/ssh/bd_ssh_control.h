#ifndef __BD_SSH_CONTROL_H__
#define __BD_SSH_CONTROL_H__

#include <string>
#include "bd_ssh.h"


using namespace std;
using namespace netprotocol;

class BD_EXPORT_CLASS bd_ssh_control
{
public:
	bd_ssh_control(const string& serverurl);
    bd_ssh_control(const string &serverurl, const string &forthParameter);
    bd_ssh_control(){}
	/**
	 * 认证请求
	 * @param username      用户名
	 * @param pwd   		密码
	 */
	void authentication(const string& username,const string& pwd)throw(bd_net_exception,std::runtime_error);
	/**
	 * 重启设备
	 */
    string reboot();
	/**
     * 关闭设备
	 */
    string shutdown();
    /**
     * 重启服务
     */
    string restartservice();
    /**
     * 停止服务
     */
    string stopservice();
    /**
     * 时间同步,机器需要安装ntpdate
     */
    string syntime();
private:
    string m_serverurl;
    string m_username;
    string m_password;
    string m_forth;
    bd_ssh m_ssh;
	//Test test;
};

#endif

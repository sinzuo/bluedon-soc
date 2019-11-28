#include "bd_ssh_control.h"
#include <iostream>


bd_ssh_control::bd_ssh_control(const string& serverurl):
		m_serverurl(serverurl)
{
}

bd_ssh_control::bd_ssh_control(const string &serverurl, const string &forthParameter)
{    
    m_serverurl = serverurl;
    m_forth = forthParameter;
}

void bd_ssh_control::authentication(const string& username,const string& pwd)throw(bd_net_exception,std::runtime_error)
{
    m_username = username;
    m_password = pwd;
	try
	{
		m_ssh.login(m_serverurl,m_username,m_password);
	}
	catch(...)
	{
		throw;
	}
}

string bd_ssh_control::reboot()
{
    string res = "0";
    try
	{
		m_ssh.command("reboot");
	}
	catch(bd_net_exception ex)
	{
        cerr << ex.what() << endl;
        return ex.what();;
	}

    return res;
}

string bd_ssh_control::shutdown()
{
    string res = "0";
    try
	{
		m_ssh.command("poweroff");
	}
	catch(bd_net_exception ex)
	{
        cerr << ex.what() << endl;
        return ex.what();;
    }

    return res;
}

string bd_ssh_control::restartservice()
{
    string res = "0";
    if(m_forth.compare("") == 0) return "para error";

    string cmd = "service";
    cmd += " ";
    cmd += m_forth;
    cmd += " ";
    cmd += "restart";
    try
    {
        m_ssh.command(cmd.c_str());
    }
    catch(bd_net_exception ex)
    {
        cerr << ex.what() << endl;
        return ex.what();;
    }

    return res;
}

string bd_ssh_control::stopservice()
{
    string res = "0";
    if(m_forth.compare("") == 0) return "para error";

    string cmd = "service";
    cmd += " ";
    cmd += m_forth;
    cmd += " ";
    cmd += "stop";
    try
    {
        m_ssh.command(cmd.c_str());
    }
    catch(bd_net_exception ex)
    {
        cerr << ex.what() << endl;
        return ex.what();;
    }

    return res;
}

string bd_ssh_control::syntime()
{
    string res = "0";
    if(m_forth.compare("") == 0) return "para error";

    string cmd = "ntpdate -u";
    cmd += " ";
    cmd += m_forth;

    try
    {
        m_ssh.command(cmd.c_str());
    }
    catch(bd_net_exception ex)
    {
        cerr << ex.what() << endl;
        return ex.what();;
    }

    return res;
}//*/

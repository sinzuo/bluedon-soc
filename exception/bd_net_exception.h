
#ifndef __BD_NET_EXCEPTION_H__
#define __BD_NET_EXCEPTION_H__
#include "bd_exception.h"
#include <string>

class BD_EXPORT_CLASS bd_net_exception: public bd_exception
{
public:

	bd_net_exception(const char* arg):m_errorcode(0),m_msg(arg)
	{

	}

	bd_net_exception(const std::string& arg):m_errorcode(0),m_msg(arg)
	{

	}

	bd_net_exception(const char* arg,int errorcode):m_errorcode(errorcode),m_msg(arg)
	{

	}

	virtual ~bd_net_exception() throw()
	{

	}

	int getcode() const
	{
		return m_errorcode;
	}

	virtual E_LEVEL getlevel() const
	{
		return L_ERROR;
	}

	virtual const char *what() const throw()
	{
		if (m_msg.empty())
			return "";

		return m_msg.c_str();
	}
private:
	int m_errorcode;
	std::string	m_msg;		///<< 异常信息
	E_LEVEL		m_level;	///<< 异常等级
};

#endif

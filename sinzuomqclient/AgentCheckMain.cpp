
#include "AgentCheckOptions.h"
#include <curl/curl.h>
#include <stdio.h>
#include "unistd.h"
#include <iostream>
#include "../utils/bdstring.h"
#include "zmq.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/propertyconfigurator.h>

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace Poco;

LoggerPtr logger_file(Logger::getLogger("file")); //获取配置文件中file对应的句柄
//LoggerPtr logger_file(Logger::getLogger("console"));//获取配置文件中console对应的句柄

tag_controller_config_t m_controllerConfig;
string m_strConfigName = "filelog.cfg";

bool LoadConfig(void)
{
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try
	{
		pConf = new IniFileConfiguration(m_strConfigName);
	}
	catch (NotFoundException &exc)
	{
		cerr << "MONITOR LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
		return false;
	}
	catch (SyntaxException &exc)
	{
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
		return false;
	}
	catch (Poco::Exception &exc)
	{
		cerr << "MONITOR LoadConfig:" << exc.displayText() << endl;
		return false;
	}

	try
	{
		GET_CONF_ITEM_CHAR(log4configfile, m_controllerConfig.chLog4File, 100);
		GET_CONF_ITEM_INT(moduleid, m_controllerConfig.nModuleId);
		GET_CONF_ITEM_CHAR(modulename, m_controllerConfig.chModuleName, 20);
		GET_CONF_ITEM_INT(moduleversion, m_controllerConfig.wModuleVersion);
		GET_CONF_ITEM_CHAR(record_separator, m_controllerConfig.chRecordSep, 2);
		GET_CONF_ITEM_CHAR(field_separator, m_controllerConfig.chFieldSep, 2);
		GET_CONF_ITEM_INT(tasknum_perthread, m_controllerConfig.nTaskNumPerThread);
		if (m_controllerConfig.nTaskNumPerThread <= 0)
		{
			m_controllerConfig.nTaskNumPerThread = 5; // 默认每个线程处理5个IP
		}
	}
	catch (NotFoundException &exc)
	{
		cerr << "MONITOR LoadConfig:" << exc.displayText() << endl;
		return false;
	}
	catch (SyntaxException &exc)
	{
		cerr << "MONITOR LoadConfig:" << exc.displayText() << endl;
		return false;
	}
	return true;
}

void printConfig()
{
	CONTROLLER_INFO_S(<= == == == == netservice configure information == == == == = >);
	CONTROLLER_INFO_V(" log4configfile=>    [" + string(m_controllerConfig.chLog4File) + string("]"));
	char chModuleId[10] = {0};
	sprintf(chModuleId, "%d", m_controllerConfig.nModuleId);
	CONTROLLER_INFO_V(" moduleid=>          [" + string(chModuleId) + string("]"));
	CONTROLLER_INFO_V(" modulename=>        [" + string(m_controllerConfig.chModuleName) + string("]"));
	char chVersion[10] = {0};
	sprintf(chVersion, "%d", m_controllerConfig.wModuleVersion);
	CONTROLLER_INFO_V(" moduleversion=>     [" + string(chVersion) + string("]"));
	CONTROLLER_INFO_V(" record_separator=>  [" + string(m_controllerConfig.chRecordSep) + string("]"));
	CONTROLLER_INFO_V(" field_separator=>   [" + string(m_controllerConfig.chFieldSep) + string("]"));
	char chTemp[5] = {0};
	sprintf(chTemp, "%d", m_controllerConfig.nTaskNumPerThread);
	CONTROLLER_INFO_V(" tasknum_perthread=> [" + string(chTemp) + string("]"));
	CONTROLLER_INFO_S(<= == == == == netservice configure information == == == == = >);
}

int main()
{

	void *m_zmq_context = zmq_ctx_new();

	void *m_zmq_client = zmq_socket(m_zmq_context, ZMQ_DEALER);

	zmq_connect(m_zmq_client, "tcp://127.0.0.1:6555");

	std::string ostr="jiangyibo";

	while (true)
	{

		
		int rec = zmq_send(m_zmq_client, ostr.c_str(), ostr.length(), 1);
		cout << "send to server with content =>[" << "strOut" << "]" << endl;
		cout << "ZMQ发送结果: [" << rec << "]" << endl;

		sleep(10000);
	}

	return 0;
}

#ifndef _AGENT_CHECK_OPTIONS_H_
#define _AGENT_CHECK_OPTIONS_H_

#include "Poco/Types.h"
#include "Poco/Mutex.h"
#include "Poco/SharedLibrary.h"
#include "Poco/Types.h"
#include "Poco/Environment.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/String.h"
#include "Poco/Util/IniFileConfiguration.h"
#include "Poco/AutoPtr.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/Thread.h"
#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"
#include "Poco/Exception.h"
#include "Poco/DynamicAny.h"
#include "Poco/UnicodeConverter.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/UTF16Encoding.h"
#include "Poco/ASCIIEncoding.h"
#include "Poco/TextEncoding.h"
#include "Poco/TextConverter.h"
#include "Poco/MD5Engine.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>

using namespace std;
using namespace Poco;
using Util::IniFileConfiguration;
using Util::AbstractConfiguration;


typedef struct BDAgentCheckTaskConfigEx { 
	char chtype[8];
	char chCheckName[128];
	char chFilterName[128];
	char chCommand[128];
}tag_check_task_config_t,*tag_check_task_config_pt;

typedef struct BDAgentCheckConfigEx { 
	//char chTaskName[8];
	//unsigned int  unCheckTaskNum;
	unsigned int  unIntevalSecond;
    vector<tag_check_task_config_t> vecTaskCfg;
}tag_check_config_t,*tag_check_config_pt;


#define CHECK_TYPE_EXE  "E"          // executable file
#define CHECK_TYPE_URL  "U"          // url address


#define GET_CONF_ITEM_STRING(from,to)\ 
		if( pConf->hasProperty(from) ) {\
			string strValue = pConf->getString(from);\
			to = strValue;\		
 		} else {\
 		    cerr<<"error: no configuration item =>"<<from<<endl;\
 		    return false;\
		}  

#define GET_CONF_ITEM_INT(from,to)\ 
		if( pConf->hasProperty(from) ) {\
			int nValue = pConf->getInt(from);\
			to = nValue;\		
 		} else {\
 		    cerr<<"error: no configuration item =>"<<from<<endl;\
 		    return false;\
		}

#define GET_CONF_ITEM_CHAR(from,to,size)\ 
		if( pConf->hasProperty(from) ) {\
			string strValue = pConf->getString(from);\		
			strncpy(to,strValue.c_str(),size);\	
 		} else {\
 		    cerr<<"error: no configuration item =>"<<from<<endl;\
 		    return false;\
		}  

#define GET_CONF_ITEM_BOOL(from,to)\ 
		if( pConf->hasProperty(from) ) {\
			bool bValue = pConf->getBool(from);\
			to = bValue;\		
 		} else {\
 		    cerr<<"error: no configuration item =>"<<from<<endl;\
 		    return false;\
		}  
		
class CAgentCheckOptions
{
public:
	CAgentCheckOptions();
	~CAgentCheckOptions();

public:
	static CAgentCheckOptions * instance();
	void CommondParse(char argc, char **argv);	
	void Init();
	void PrintUsage();
 
public:
	inline const tag_check_config_t * GetConfig()  {return &m_checkConfig;}
	void CommandExecute(const char *command);

private:
	void Daemon();
	bool Load();
	bool LoadBaseCfg();

private:
	static CAgentCheckOptions * m_instance;
	string m_strAppConfigFile;
	bool m_bdaemon;
	tag_check_config_t m_checkConfig;
	
};

#endif //_AGENT_CHECK_OPTIONS_H_



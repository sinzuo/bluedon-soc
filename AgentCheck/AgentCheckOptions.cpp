
#include "AgentCheckOptions.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include  "unistd.h"


using namespace Poco;
using namespace std;


CAgentCheckOptions  * CAgentCheckOptions::m_instance=NULL;


CAgentCheckOptions::CAgentCheckOptions()
{
    m_checkConfig.unIntevalSecond = 0;
	m_checkConfig.vecTaskCfg.clear();
	m_bdaemon = true;
}

CAgentCheckOptions::~CAgentCheckOptions()
{
	if(m_instance!=NULL) {
		delete m_instance;
		m_instance=NULL;
	}
}


CAgentCheckOptions * CAgentCheckOptions::instance()
{
	if(m_instance==NULL) {
        if(m_instance==NULL)
            m_instance = new CAgentCheckOptions();
    }

    return m_instance;
}

void CAgentCheckOptions::Init()
{ 
    cout<<" Init start ... "<<endl;
	if ( !Load() ) { 
		cerr<<" error: Load() failed !"<<endl;
        return;
	}
    return;
}

void CAgentCheckOptions::PrintUsage() {
    printf("Usage : [agent_check] [-h] [-c configure][-e]\n");
    printf("        -h Show help\n");
    printf("        -c configure file\n");
    printf("        -e don't start as daemon process\n");
    printf("For example:\n");
    printf("agent_check -c /root/SOC/soc5.0/confagent_check.ini\n");
}

void CAgentCheckOptions::CommondParse(char argc, char **argv)
{
	short int ch;            //asiic value of char 
	char optstr[] = "ehc:";  //format of program command line
	
	while((ch = getopt(argc, argv, optstr)) != -1 ) {
        switch(ch) {
            case 'h':
                PrintUsage();
                exit(-1);
            case 'c':
                m_strAppConfigFile.assign(optarg); //configure file
                break;
            case 'e':
        	    m_bdaemon = false;  //not daemon
                break;
            case '?':
            	printf("unknown parameter: %c\n", optopt);
                PrintUsage();
                exit(-1);
            case ':':
            	printf("need parameter: %c\n", optopt);
                PrintUsage();            
                exit(-1);
            default:
        	    PrintUsage();
        	    exit(-1);
        }
    }
	if(access(m_strAppConfigFile.c_str(), F_OK) != 0) {
	    printf("configfile %s not exist,please check!\n", m_strAppConfigFile.c_str());
	    PrintUsage();
	    exit(-1);
	}
	
	if(m_bdaemon) Daemon(); //Deamon process...

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
}


void CAgentCheckOptions::Daemon() {
    int pid = 0;
	//umask(0);
    if((pid=fork()) > 0) exit(0);
    setsid();
    if((pid=fork()) > 0) exit(0);
	cout<<"start as Daemon..."<<endl;
}

bool CAgentCheckOptions::Load()
{
	if ( !LoadBaseCfg() ) { 
		cerr<<" error: LoadBaseCfg() failed !"<<endl;
        return false;
	}

    return true;
}

bool CAgentCheckOptions::LoadBaseCfg()
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strAppConfigFile);
	} catch (NotFoundException& exc ) {
		cerr << "LoadBaseCfg IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadBaseCfg IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
		cerr << "LoadBaseCfg:" << exc.displayText() << endl;	
		return false;
	}

	try {
        unsigned int unNum = 0;
        GET_CONF_ITEM_INT("COMMON.task_num",unNum);
		if ( unNum <= 0 ) return true;
		GET_CONF_ITEM_INT("COMMON.inteval_second",m_checkConfig.unIntevalSecond);
		if (m_checkConfig.unIntevalSecond < 30) {
			m_checkConfig.unIntevalSecond = 30;
		}
		cout<<"COMMON.task_num =>["<<unNum<<"]"<<endl;
		cout<<"COMMON.inteval_second =>["<<m_checkConfig.unIntevalSecond<<"]"<<endl;
		for( int i=1; i <= unNum; i++ ) {
			char chTaskIndex[3] = {0};
			sprintf(chTaskIndex, "%u", i);
			string strTaskName = "TASK";
			strTaskName += chTaskIndex;

			tag_check_task_config_t tag_temp;
			memset(&tag_temp, 0, sizeof(tag_check_task_config_t));
			GET_CONF_ITEM_CHAR((strTaskName  + ".type").c_str() ,tag_temp.chtype, 1);
			if ( strncmp(tag_temp.chtype,CHECK_TYPE_EXE,1) != 0
				&& strncmp(tag_temp.chtype,CHECK_TYPE_URL,1) != 0) {
				cout<<"config error: invalid type => ["<<tag_temp.chtype<<"]"<<endl;
				return false;
			}
			GET_CONF_ITEM_CHAR((strTaskName + ".check_name").c_str()  ,tag_temp.chCheckName  ,128-1);
			GET_CONF_ITEM_CHAR((strTaskName + ".filter_name").c_str() ,tag_temp.chFilterName ,128-1);
			GET_CONF_ITEM_CHAR((strTaskName + ".command").c_str()     ,tag_temp.chCommand    ,128-1);

			m_checkConfig.vecTaskCfg.push_back(tag_temp);
		}
 	}
	catch (NotFoundException& exc ) {
		cerr << "LoadBaseCfg NotFoundException:" << exc.displayText() << endl;	
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadBaseCfg SyntaxException:" << exc.displayText() << endl;	
		return false;
	}
    return true;
}

void CAgentCheckOptions::CommandExecute(const char *command) {

    int status = system(command);
    if ( status < 0) {
	    printf("error: %s failed => %s",command, strerror(errno));
    }
	return;
}


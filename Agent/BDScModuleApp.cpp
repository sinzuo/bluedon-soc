
#include "BDScModuleApp.h"
#include "zmq.h"
#include "utils/bdstring.h"
#include "probuf/socmsg.pb.h"

#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <fcntl.h>

using namespace std;
using Poco::DeflatingOutputStream;
using Poco::DeflatingStreamBuf;
using Poco::InflatingInputStream;
using Poco::InflatingStreamBuf;
//using namespace log4cxx;

extern CBDScModuleApp *g_pApp;   
CBDScModuleApp CBDScModuleApp::m_App;
unsigned int CBDScModuleApp::m_nLog = 0;

CBDScModuleApp::CBDScModuleApp() {
    g_pApp          = this;
    m_bdaemon       = true;   //是否为守护进程
    m_registered    = false;  // default no register
    m_isCompressed  = false;  //是否压缩
    m_isEncrypted   = false;  //是否加密
    m_nCompressType = Message_comtypeNo_NO_COMPRESS; //压缩模式
    m_nEncryptType  = Message_enptypeNo_NO_ENPTYPE; //加密模式
    m_listreq.clear();
    m_nTimeSleepHeartBeat   = 6000000; // microsecond
	m_nTimeMonitorHeartBeat = 10;      // second
    m_MessageObj.clear();
	m_ZmqObj.clear();
    //zmq句柄
    m_agentConfigPtr   = NULL;
	m_zmq_context      = NULL;
	m_zmq_server       = NULL;
	m_zmq_requester    = NULL;
	m_zmq_responseterA = NULL;
	m_zmq_responseterB = NULL;
	m_zmq_responseterC = NULL; 
    m_zmq_responseterD = NULL;
    m_zmq_responseterE = NULL;
    //probuf消息体指针
	m_pbHeartbeatWritePtr     = NULL;
	m_pbHeartbeatReadPtr      = NULL;
	m_pbServerPtr             = NULL;
	m_pbASWritePtr_syslog     = NULL;
	m_pbASWritePtr_filelog    = NULL;
	m_pbASWritePtr_snmptrap   = NULL;
	m_pbASWritePtr_ping       = NULL;
	m_pbASWritePtr_scanport   = NULL;
	m_pbASWritePtr_inspector  = NULL;
	m_pbASWritePtr_monitor    = NULL;
    m_pbASWritePtr_controller = NULL;
    m_pbASWritePtr_snmpcheck  = NULL;
    m_pbASWritePtr_bdsar      = NULL;
    m_pbASWritePtr_openvas    = NULL;
    m_pbASWritePtr_sftp       = NULL;
    m_pbASWritePtr_threatint  = NULL;
    m_pbASWritePtr_topu       = NULL;
    m_pbASWritePtr_wmi        = NULL;
    m_pbASWritePtr_nmapscan   = NULL;
    m_pbASWritePtr_netflow    = NULL;
    m_pbASWritePtr_inspecUrl  = NULL;
    m_pbASWritePtr_weakpasswd = NULL;
    m_pbASWritePtr_flow       = NULL;
}
//释放资源
CBDScModuleApp::~CBDScModuleApp() {

	DEL_PTR(m_pbHeartbeatWritePtr);
	DEL_PTR(m_pbHeartbeatReadPtr);
	DEL_PTR(m_pbServerPtr);
	DEL_PTR(m_pbASWritePtr_syslog);
	DEL_PTR(m_pbASWritePtr_filelog);
	DEL_PTR(m_pbASWritePtr_snmptrap);
	DEL_PTR(m_pbASWritePtr_ping);
	DEL_PTR(m_pbASWritePtr_scanport);	
	DEL_PTR(m_pbASWritePtr_inspector);
	DEL_PTR(m_pbASWritePtr_monitor);
    DEL_PTR(m_pbASWritePtr_controller);
    DEL_PTR(m_pbASWritePtr_snmpcheck);
    DEL_PTR(m_pbASWritePtr_bdsar);
    DEL_PTR(m_pbASWritePtr_openvas);
    DEL_PTR(m_pbASWritePtr_sftp);
    DEL_PTR(m_pbASWritePtr_threatint);
    DEL_PTR(m_pbASWritePtr_topu);
    DEL_PTR(m_pbASWritePtr_wmi);
    DEL_PTR(m_pbASWritePtr_nmapscan);
    DEL_PTR(m_pbASWritePtr_netflow);
    DEL_PTR(m_pbASWritePtr_flow);
    DEL_PTR(m_pbASWritePtr_inspecUrl);
    DEL_PTR(m_pbASWritePtr_weakpasswd);

	DEL_ZMQ(m_zmq_requester);
	DEL_ZMQ(m_zmq_responseterA);
	DEL_ZMQ(m_zmq_responseterB);
	DEL_ZMQ(m_zmq_responseterC); 
    DEL_ZMQ(m_zmq_responseterD);
    DEL_ZMQ(m_zmq_responseterE);
	DEL_ZMQ(m_zmq_server);
	
    if(m_zmq_context != NULL) {	
		zmq_ctx_destroy (m_zmq_context);
		m_zmq_context = NULL;
    }
}
//打印使用方法
void CBDScModuleApp::PrintUsage() {
    printf("Usage : [filemonproxy ] [-h] [-c configure][-e]\n");
    printf("        -h Show help\n");
    printf("        -c configure file\n");
    printf("        -e don't start as daemon process\n");
    printf("For example:\n");
    printf("filemonproxy -c ../etc/filemonproxy.conf\n");
}

int CBDScModuleApp::CreatePidFile(const char * pidfile){
    char val[16] = {0};
    size_t len = snprintf(val, sizeof(val), "%d", getpid());
    int pidfd = 0;
    pidfd = open(pidfile, O_CREAT | O_TRUNC | O_NOFOLLOW | O_WRONLY, 0644);
    if (pidfd < 0) {
        return -1;
    }
    size_t r = write(pidfd, val, (unsigned int)len);
    if (r != len) {
        return -1;
    }
    close(pidfd);
    return 0;

}

//设置为守护进程
void CBDScModuleApp::Daemon() {
    int pid = 0;
//    chdir("/home/SOC/soc5.1/bin");
	//umask(0);
    if ((pid=fork())  >  0) {
        exit(0);
    }
    setsid();
    if((pid=fork()) > 0) {
        exit(0);
    }

    if (CreatePidFile("/var/run/BdSocAgent.pid") < 0) {
        exit(0);
    }

    int fd = open("/dev/null", O_RDWR, 0);
//    int fd = open("/home/SOC/soc5.1/log/daemon.log", O_RDWR | O_CREAT, 0);

    if (fd < 0) {
        exit(0);
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        exit(0);
    }
    if (dup2(fd, STDERR_FILENO) < 0) {
        exit(0);
    }
}

//读取参数
void CBDScModuleApp::CommondParse(char argc, char **argv)
{
	short int ch;            //asiic value of char 
	char optstr[] = "ehc:";  //format of program command line
	
	while((ch = getopt(argc, argv, optstr)) != -1 ) {
        switch(ch) {
            case 'h':
                PrintUsage();
                exit(-1);
            case 'c':
                m_strcfgname.assign(optarg); //configure file
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
	if(access(m_strcfgname.c_str(), F_OK) != 0) {
	    printf("configfile %s not exist,please check!\n", m_strcfgname.c_str());
	    PrintUsage();
	    exit(-1);
	}
	if(m_bdaemon) Daemon(); //Deamon process...
}

//initlize log and configure
int CBDScModuleApp::InitOptions() {
	if(GetCBOptions() == NULL) {
	    printf("CBDScModuleApp::InitOptions() Failed...\n");
		return -1;
	} else {
		GetCBOptions()->SetAppConfig(m_strcfgname);
        GetCBOptions()->Init();
	    if( !GetCBOptions()->Start() )  return -1;
	}
	return 0;
}

//模块初始化
int CBDScModuleApp::InitModuleMgr() {
	
	if(GetBDScModuleMgr() == NULL) { 
		AGENTLOG_ERROR_S(CBDScModuleApp::InitModuleMgr() Failed...);
		return -1;
	} else {
		GetBDScModuleMgr()->Init();
		if( !GetBDScModuleMgr()->Start())  return -1;
	}
	return 0;
}

//初始化protobuf
int CBDScModuleApp::InitProBuf() {
    try {
        if (m_pbHeartbeatWritePtr == NULL)     m_pbHeartbeatWritePtr       = new Message();
        if (m_pbHeartbeatReadPtr == NULL)      m_pbHeartbeatReadPtr        = new Message();
        if (m_pbServerPtr == NULL)             m_pbServerPtr               = new Message();

        PB_SEND_MSG_INI(m_pbASWritePtr_syslog, SYSLOG, Message_ServiceNo_LOG, MODULE_DATA_LOG_SYSLOG);
        PB_SEND_MSG_INI(m_pbASWritePtr_filelog, FILELOG, Message_ServiceNo_LOG, MODULE_DATA_LOG_FILELOG);
        PB_SEND_MSG_INI(m_pbASWritePtr_snmptrap, SNMPTRAP, Message_ServiceNo_LOG, MODULE_DATA_LOG_SNMPTRAP);
        PB_SEND_MSG_INI(m_pbASWritePtr_wmi, WMI, Message_ServiceNo_LOG, MODULE_DATA_LOG_WMI);
        PB_SEND_MSG_INI(m_pbASWritePtr_sftp, SFTP, Message_ServiceNo_LOG, MODULE_DATA_LOG_SFTP);
        PB_SEND_MSG_INI(m_pbASWritePtr_ping, PING, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_PING);
        PB_SEND_MSG_INI(m_pbASWritePtr_scanport, SCANPORT, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_SCANPORT);
        PB_SEND_MSG_INI(m_pbASWritePtr_inspector, INSPECTOR, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_INSPECTOR);
        PB_SEND_MSG_INI(m_pbASWritePtr_monitor, MONITOR, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_MONITOR);
        PB_SEND_MSG_INI(m_pbASWritePtr_controller,CONTROLLER, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_CONTROLLER);
        PB_SEND_MSG_INI(m_pbASWritePtr_snmpcheck,SNMPCHECK, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_SNMPCHECK);
        PB_SEND_MSG_INI(m_pbASWritePtr_weakpasswd, WEAKPASSWD,Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_WEAKPASSWD);
        PB_SEND_MSG_INI(m_pbASWritePtr_topu, NETTOP, Message_ServiceNo_SERVICE, MODULE_DATA_SERVICE_NETTOPO);

        PB_SEND_MSG_INI(m_pbASWritePtr_nmapscan, NMAPSCAN, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_NMAPSCAN);
        PB_SEND_MSG_INI(m_pbASWritePtr_bdsar, NETTOP, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_BDSAR);
        PB_SEND_MSG_INI(m_pbASWritePtr_openvas, LEAKSCAN, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_OPENVAS);
//        PB_SEND_MSG_INI(m_pbASWritePtr_sftp, SFTP, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_SFTP);
        PB_SEND_MSG_INI(m_pbASWritePtr_threatint, THREATINT, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_THREATINT);
        PB_SEND_MSG_INI(m_pbASWritePtr_inspecUrl, INSPECURL, Message_ServiceNo_PERFORMANCE, MODULE_DATA_PERFORMANCE_INSPECURL);
        PB_SEND_MSG_INI(m_pbASWritePtr_netflow, NETFLOW, Message_ServiceNo_NETFLOW, MODULE_DATA_FLOW_NETFLOW);
        PB_SEND_MSG_INI(m_pbASWritePtr_flow, FLOW, Message_ServiceNo_FLOW, MODULE_DATA_FLOW_FLOW);

    } catch(bad_alloc &) {
		AGENTLOG_ERROR_S(CBDScModuleApp::InitModuleMgr() Failed...);
		return -1;
	}		
    return 0;
}

//初始化zmq
int CBDScModuleApp::InitZMQ() {

    int major, minor, patch;
    zmq_version (&major, &minor, &patch);
	char zmq_ver[50] = {0};
    sprintf (zmq_ver,"Current 0MQ version is %d.%d.%d", major, minor, patch);
	AGENTLOG_INFO_V(string(zmq_ver));
    //创建上下文
	if( (m_zmq_context = zmq_ctx_new()) == NULL ) {
  		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
		return -1;
	}
	//设置线程数
	if( zmq_ctx_set (m_zmq_context, ZMQ_IO_THREADS, 5) != 0)
	{
		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() zmq_ctx_set Failed...);
		return -1;
	}
	//REP to MS
    //作为服务端接收数据
	if( (m_zmq_server = zmq_socket(m_zmq_context, ZMQ_REP)) == NULL ) {
  		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
		return -1;
	} else {
		string strServerBindAddr;
		string strServerBindProtocol;
		string strServerBindIp;
		string strServerBindPort;
		
		strServerBindProtocol = m_agentConfigPtr->chCollectServerProtocol; 
		if (strServerBindProtocol.empty()) {
			strServerBindProtocol = "tcp";
		}
		strServerBindProtocol += "://";
		
		strServerBindIp = m_agentConfigPtr->chCollectServerIP; 
		if (strServerBindIp.empty()) {
			strServerBindIp = "localhost";
		}
		int nPort = m_agentConfigPtr->nCollectServerPort; 
		if (m_agentConfigPtr->nCollectServerPort <= 0) {
			strServerBindPort = "5556";
		}
		char chtemp[10]={0};
		sprintf(chtemp,"%d",nPort);
		strServerBindPort = chtemp;

		strServerBindAddr = strServerBindProtocol 
			              + strServerBindIp
			              + ":"
			              + strServerBindPort;
			 	
        if( zmq_bind (m_zmq_server, strServerBindAddr.c_str()) != 0){
			AGENTLOG_ERROR_V("zmq_bind " + strServerBindAddr + " Failed!");	
			return -1;
		}
		AGENTLOG_INFO_V("zmq_bind " + strServerBindAddr + " Successed!");
    }

	//REQ to ROUTER
	if( (m_zmq_requester = zmq_socket(m_zmq_context, ZMQ_REQ)) == NULL ) {
  		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
		return -1;
	} else {
		string strTmp = m_agentConfigPtr->chReqServAddr; 
		if (!strTmp.empty()) { 
			zmq_connect(m_zmq_requester, strTmp.c_str());
		} else {
			AGENTLOG_ERROR_S(MS Address String is empty!);
			return -1;
		}
    }
    //Asynchronous client-to-server (XREQ to XREP)
	if( (m_zmq_responseterA = zmq_socket(m_zmq_context, ZMQ_XREQ)) == NULL ) {
		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
		return -1;
	} else {
        //设置日志最大缓存
        int affinity = 50000;
        zmq_setsockopt(m_zmq_responseterA, ZMQ_SNDHWM, &affinity, sizeof (affinity));
        zmq_setsockopt(m_zmq_responseterA, ZMQ_RCVHWM, &affinity, sizeof (affinity));
        affinity = 65535;
        zmq_setsockopt(m_zmq_responseterA, ZMQ_SNDBUF, &affinity, sizeof (affinity));
        zmq_setsockopt (m_zmq_responseterA, ZMQ_RCVBUF, &affinity, sizeof (affinity));
		string strTmp = m_agentConfigPtr->chRespServAddrA; 
		if (!strTmp.empty()) { 
			zmq_connect(m_zmq_responseterA, strTmp.c_str());
		} else {
			AGENTLOG_ERROR_S(AS Address String is empty!);
			return -1;
		}
    }
    m_ZmqObj[MODULE_DATA_LOG] = m_zmq_responseterA;
	
    //Asynchronous client-to-server (XREQ to XREP)
	if( (m_zmq_responseterB = zmq_socket(m_zmq_context, ZMQ_XREQ)) == NULL ) {
		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
		return -1;
	} else {
		string strTmp = m_agentConfigPtr->chRespServAddrB; 
		if (!strTmp.empty()) { 
			zmq_connect(m_zmq_responseterB, strTmp.c_str());
		} else {
			AGENTLOG_ERROR_S(AS Address String is empty!);
			return -1;
		}
    }
	m_ZmqObj[MODULE_DATA_PERFORMANCE] = m_zmq_responseterB;
	
    //Asynchronous client-to-server (XREQ to XREP)
	if( (m_zmq_responseterC = zmq_socket(m_zmq_context, ZMQ_XREQ)) == NULL ) {
		AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
		return -1;
	} else {
		string strTmp = m_agentConfigPtr->chRespServAddrC; 
		if (!strTmp.empty()) { 
			zmq_connect(m_zmq_responseterC, strTmp.c_str());
		} else {
			AGENTLOG_ERROR_S(AS Address String is empty!);
			return -1;
		}
    }
	m_ZmqObj[MODULE_DATA_SERVICE] = m_zmq_responseterC;

    //Asynchronous client-to-server (XREQ to XREP)
    if( (m_zmq_responseterD = zmq_socket(m_zmq_context, ZMQ_XREQ)) == NULL ) {
        AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
        return -1;
    } else {
        string strTmp = m_agentConfigPtr->chRespServAddrD;
        if (!strTmp.empty()) {
            zmq_connect(m_zmq_responseterD, strTmp.c_str());
        } else {
            AGENTLOG_ERROR_S(AS Address String is empty!);
            return -1;
        }
    }
    m_ZmqObj[MODULE_DATA_FLOW] = m_zmq_responseterD;

    //Asynchronous client-to-server (XREQ to XREP)
    if( (m_zmq_responseterE = zmq_socket(m_zmq_context, ZMQ_XREQ)) == NULL ) {
        AGENTLOG_ERROR_S(CBDScModuleApp::InitZMQ() Failed...);
        return -1;
    } else {
        string strTmp = m_agentConfigPtr->chRespServAddrE;
        if (!strTmp.empty()) {
            zmq_connect(m_zmq_responseterE, strTmp.c_str());
        } else {
            AGENTLOG_ERROR_S(AS Address String is empty!);
            return -1;
        }
    }
    m_ZmqObj[MODULE_DATA_NETFLOW] = m_zmq_responseterE;

    return 0;
}

//初始化获取系统信息
int CBDScModuleApp::InitBaseSysInfo() {
	if(GetCBDBaseSysInfo() == NULL) {
		AGENTLOG_ERROR_S(CBDScModuleApp::InitBaseSysInfo() Failed...);
		return -1;
	} 
    return 0;
}

//初始化入口
int CBDScModuleApp::Initlize() {

    setlocale(LC_ALL, ""); //设置log4, 防止中文乱码
	if( InitOptions()   != 0 ) return -1;	//initlize log and configure 
    m_agentConfigPtr = GetCBOptions()->GetConfig();
    if(m_agentConfigPtr->nHeartbeatSleepnanosec > 0) {
        m_nTimeSleepHeartBeat = m_agentConfigPtr->nHeartbeatSleepnanosec;
    }
    if(m_agentConfigPtr->nHeartbeatMonitorsec > 0) {
		m_nTimeMonitorHeartBeat = m_agentConfigPtr->nHeartbeatMonitorsec;
    }
    if(m_agentConfigPtr->nEncryptControl) { //设置加密及默认加密方式
        m_isEncrypted = true;
        m_nEncryptType = Message_enptypeNo_AES256ECB;
    }
    if(m_agentConfigPtr->nCompressControl) { //设置压缩及默认压缩方式
        m_isCompressed = true;
        m_nCompressType = Message_comtypeNo_COMPRESS_ZLIB;
    }
	
	AGENTLOG_INFO_S(initlize start...);
    if( InitModuleMgr()   != 0 ) return -1;	//initlize CBDScModuleMgr 
	if( InitProBuf()      != 0 ) return -1; //initlize ProtocalBuffer
	if( InitZMQ()         != 0 ) return -1; //initlize ZeroMQ 
	if( InitBaseSysInfo() != 0 ) return -1; //initilize InitBaseSysInfo
	AGENTLOG_INFO_S(initlize end...);


	printConfig(); // print config

    return 0;
}

//打印配置
void CBDScModuleApp::printConfig() {
	AGENTLOG_INFO_S(<===========agent configure information===========>);
	AGENTLOG_INFO_V(" collect_server_ip=>      ["+string(m_agentConfigPtr->chCollectServerIP)+string("]"));
	AGENTLOG_INFO_V(" collect_server_name=>    ["+string(m_agentConfigPtr->chCollectServerName)+string("]"));
	char chPort[6] = {0};
	sprintf(chPort,"%d",m_agentConfigPtr->nCollectServerPort);
	AGENTLOG_INFO_V(" collect_server_port=>    ["+string(chPort)+string("]"));
	char chNanoTime[10] = {0};
	sprintf(chNanoTime,"%d",m_agentConfigPtr->nHeartbeatSleepnanosec);
	char chMonitorTime[5] = {0};
	sprintf(chMonitorTime,"%d",m_agentConfigPtr->nHeartbeatMonitorsec);
	AGENTLOG_INFO_V(" collect_server_port=>    ["+string(chPort)+string("]"));
	AGENTLOG_INFO_V(" heartbeat_sleepnanosec=> ["+string(chNanoTime)+string("]"));
	AGENTLOG_INFO_V(" heartbeat_monitorsec=>   ["+string(chMonitorTime)+string("]"));
	AGENTLOG_INFO_V(" record_separator=>       ["+string(m_agentConfigPtr->chRecordSep)+string("]"));
	AGENTLOG_INFO_V(" field_separator=>        ["+string(m_agentConfigPtr->chFieldSep)+string("]"));
	AGENTLOG_INFO_V(" log4configfile=>         ["+string(m_agentConfigPtr->chLog4File)+string("]"));
	AGENTLOG_INFO_V(" module_xml=>             ["+string(m_agentConfigPtr->chModuleXml)+string("]"));
	AGENTLOG_INFO_V(" managerserver_address=>  ["+string(m_agentConfigPtr->chReqServAddr)+string("]"));
	AGENTLOG_INFO_V(" analysisserver_address_a=>  ["+string(m_agentConfigPtr->chRespServAddrA)+string("]"));
	AGENTLOG_INFO_V(" analysisserver_address_b=>  ["+string(m_agentConfigPtr->chRespServAddrB)+string("]"));
	AGENTLOG_INFO_V(" analysisserver_address_c=>  ["+string(m_agentConfigPtr->chRespServAddrC)+string("]"));
    AGENTLOG_INFO_V(" analysisserver_address_d=>  ["+string(m_agentConfigPtr->chRespServAddrD)+string("]"));
    AGENTLOG_INFO_V(" analysisserver_address_e=>  ["+string(m_agentConfigPtr->chRespServAddrE)+string("]"));
	AGENTLOG_INFO_S(<===========agent configure information===========>);
}

//心跳发送线程
void*CBDScModuleApp::HeartBeatSend(void *arg) {
    CBDScModuleApp *cAppThis = reinterpret_cast<CBDScModuleApp*>(arg);
    while(true) {
        string strContent;
        //判断是否注册
        if(! cAppThis->m_registered) { // register
            strContent += cAppThis->m_agentConfigPtr->chCollectServerIP;
	        strContent += cAppThis->m_agentConfigPtr->chFieldSep;
	        strContent += cAppThis->m_agentConfigPtr->chCollectServerName;
	        char chPortTmp[6] = {0};
	        sprintf(chPortTmp,"%d",cAppThis->m_agentConfigPtr->nCollectServerPort);
	        strContent += cAppThis->m_agentConfigPtr->chFieldSep;
	        strContent += chPortTmp;    // client port is useless
	        strContent += cAppThis->m_agentConfigPtr->chFieldSep;
	        strContent += string("1");  // 1:linux 2:windows
	        strContent += cAppThis->m_agentConfigPtr->chFieldSep;
	        strContent += string("1");  // 1:CS(collect Server) 2:AS 3:MS 4:PS 5:WS
	        

            cAppThis->m_pbHeartbeatWritePtr->set_service_no(Message_ServiceNo_REGISTER); 
		}
        else {	// heartbeat
           //注册后发送心跳
	        cAppThis->GetBDScModuleMgr()->GetModuleStateString(strContent);
	        strContent = string(cAppThis->m_agentConfigPtr->chCollectServerIP) 
				        + cAppThis->m_agentConfigPtr->chRecordSep	   
				        + strContent;

	        cAppThis->m_pbHeartbeatWritePtr->set_service_no(Message_ServiceNo_SESSION); 

            if(cAppThis->TimeDiff() >= cAppThis->m_nTimeMonitorHeartBeat) { // reprot monitor information
                static CPU_OCCUPY cpuinfo_first; // 记录上一次cpu信息

                if(cpuinfo_first.idle == 0)// 如果是刚启动，没有上一次cpu信息，则获取一次
                {
                    cAppThis->GetCBDBaseSysInfo()->getProcCpu(cpuinfo_first);
                    sleep(1);  // for caculate cpu occupy
                }

				CPU_OCCUPY cpuinfo_second; // memset inside
				cAppThis->GetCBDBaseSysInfo()->getProcCpu(cpuinfo_second);

                int nCpuOccupy = cAppThis->GetCBDBaseSysInfo()->calCpuOccupy(cpuinfo_first,cpuinfo_second);
                //把这次的cpu信息保存，用于下次计算
                cpuinfo_first.idle = cpuinfo_second.idle;
                cpuinfo_first.iowait = cpuinfo_second.iowait;
                cpuinfo_first.irq = cpuinfo_second.irq;
                cpuinfo_first.nice = cpuinfo_second.nice;
                cpuinfo_first.softirq = cpuinfo_second.softirq;
                cpuinfo_first.system = cpuinfo_second.system;
                cpuinfo_first.user = cpuinfo_second.user;

				struct sysinfo tag_meminfo; // memset inside
				cAppThis->GetCBDBaseSysInfo()->getHostMemInfo(tag_meminfo);
                int nRamOccupy = (int)ceil((double)100*(tag_meminfo.totalram -tag_meminfo.freeram) / tag_meminfo.totalram);

                int nDiskOccupy = cAppThis->GetCBDBaseSysInfo()->getDiskOccupy();

				cAppThis->m_pbHeartbeatWritePtr->set_service_no(Message_ServiceNo_MONITOR_HOST);
                char chcpu[5] = {0};
				char chram[5] = {0};
                char chdisk[5] = {0};
				char chlog_num[10] = {0};
				sprintf(chcpu,"%d",nCpuOccupy);
				sprintf(chram,"%d",nRamOccupy);
				sprintf(chdisk,"%d",nDiskOccupy);
				unsigned int num = cAppThis->m_nLog;  // record number of sending logs		                               
				cAppThis->m_nLog = 0;
				sprintf(chlog_num,"%u",num);
				strContent = string(chcpu) + cAppThis->m_agentConfigPtr->chFieldSep 
					       + string(chram) + cAppThis->m_agentConfigPtr->chFieldSep 
                           + string(chdisk) + cAppThis->m_agentConfigPtr->chFieldSep
					       + string(chlog_num);
                strContent = string(cAppThis->m_agentConfigPtr->chCollectServerIP)
                                            + cAppThis->m_agentConfigPtr->chRecordSep
                                            + strContent;
			}
        }

        AGENTLOG_INFO_V(strContent);
        //对消息进行解密
        cAppThis->EnCodeData(strContent, cAppThis->m_isEncrypted, cAppThis->m_nEncryptType, cAppThis->m_isCompressed, cAppThis->m_nCompressType);

        cAppThis->m_pbHeartbeatWritePtr->set_content(strContent);
        cAppThis->m_pbHeartbeatWritePtr->set_enptype(cAppThis->m_nEncryptType);
        cAppThis->m_pbHeartbeatWritePtr->set_comtype(cAppThis->m_nCompressType);
        cAppThis->m_pbHeartbeatWritePtr->set_serial_no(0); 
        time_t t = time(NULL);
        cAppThis->m_pbHeartbeatWritePtr->set_time((unsigned long long)t);
		
		ostringstream ostr;
		cAppThis->m_pbHeartbeatWritePtr->SerializeToOstream(&ostr);		
	    zmq_send(cAppThis->m_zmq_requester,ostr.str().c_str(), ostr.str().length(), 0);

	    usleep(cAppThis->m_nTimeSleepHeartBeat); 
	}
	return NULL;
}

//心跳接收线程
void *CBDScModuleApp::HeartBeatRecv(void *arg) {
	AGENTLOG_INFO_S(HeartBeatRecv starts...);
	CBDScModuleApp *cAppThis = reinterpret_cast<CBDScModuleApp*>(arg);

    while(true) {
		char chReqData[1024]	= {0};
		int ret = zmq_recv(cAppThis->m_zmq_requester, chReqData, 1024, 0);
		
        if((ret < 0) || (strlen(chReqData) == 0)) {
			if( cAppThis->m_registered ) { // already registered
                /* sleep or return or break ? */				
				//AGENTLOG_DEBUG_S(received a message NULL);
				usleep(cAppThis->m_nTimeSleepHeartBeat);  
			}
			continue;
        }
		else {	
			AGENTLOG_DEBUG_V("received a message :" + string(chReqData));
			istringstream istr(chReqData);			
			if (!cAppThis->m_pbHeartbeatReadPtr->ParseFromIstream(&istr)) {
				AGENTLOG_ERROR_S(protocalbuffer parseFromIstream failed...);
                continue;
            }
			switch(cAppThis->m_pbHeartbeatReadPtr->service_no()) {
				case  Message_ServiceNo_SESSION:       // register finished
                    if (!cAppThis->m_registered) {
					    cAppThis->m_registered = true; 			
					    AGENTLOG_INFO_S(Register to MS successed!);
                    }
					break;
				case  Message_ServiceNo_REGISTER:	   // restart register 
					cAppThis->m_registered = false;			
					AGENTLOG_INFO_S(Restart register to MS !);
					break;
				default:
					AGENTLOG_WARN_S( invalid message type from MS!);
				    break;
			}
		}
    }
	return NULL;
}

//接收策略线程
void *CBDScModuleApp::Reader(void *arg) {
	AGENTLOG_INFO_S(Reader starts...);
	CBDScModuleApp *cAppThis = reinterpret_cast<CBDScModuleApp*>(arg);

    bool isEncrypted = false;
    bool isCompressed = false;
    Message_enptypeNo tmpEnType = Message_enptypeNo_NO_ENPTYPE;
    Message_comtypeNo tmpCoType = Message_comtypeNo_NO_COMPRESS;

    while(true) {
		Poco::UInt32 dwModuleIDTmp = 0;  //module id
		Poco::UInt32 dwTypeTmp     = 0;  //bussiness type
        char chReqData[200*1024]       = {0};
		bd_request_t tag_reques;
        int recSize = zmq_recv(cAppThis->m_zmq_server, chReqData, 200*1024, 0);

        if(strlen(chReqData) == 0) {		
			AGENTLOG_ERROR_S( invalid message from MS!);
            zmq_send(cAppThis->m_zmq_server,"", 0, 0); // keep client nonblock
        }		
		else {
 		    AGENTLOG_DEBUG_V("received a message :" + string(chReqData));
            string strReqData(chReqData, recSize);
            istringstream istr(strReqData);
			if (!cAppThis->m_pbServerPtr->ParseFromIstream(&istr)) {
				AGENTLOG_ERROR_S(protocalbuffer parseFromIstream failed...);	
				zmq_send(cAppThis->m_zmq_server,"", 0, 0); // keep client nonblock
                  continue;
            }
			switch(cAppThis->m_pbServerPtr->service_no()) {
				case  Message_ServiceNo_START_PLUGIN:   // start module
					dwTypeTmp  = cAppThis->m_pbServerPtr->service_no();
					AGENTLOG_DEBUG_V("message : Message_ServiceNo_START_PLUGIN");
					break;				
				case  Message_ServiceNo_STOP_PLUGIN:    // stop module
					dwTypeTmp  = cAppThis->m_pbServerPtr->service_no();
					AGENTLOG_DEBUG_V("message : Message_ServiceNo_STOP_PLUGIN");
					break;				
				case  Message_ServiceNo_START_TASK:    // stop module
					dwTypeTmp  = cAppThis->m_pbServerPtr->service_no();
					AGENTLOG_DEBUG_V("message : Message_ServiceNo_START_TASK");
					break;				
				case  Message_ServiceNo_START_AGENT:    // start agent
					dwTypeTmp  = cAppThis->m_pbServerPtr->service_no();
					AGENTLOG_DEBUG_V("message : Message_ServiceNo_START_AGENT");
					break;
				case  Message_ServiceNo_STOP_AGENT:     // stop agent
					dwTypeTmp  = cAppThis->m_pbServerPtr->service_no();
					AGENTLOG_DEBUG_V("message : Message_ServiceNo_STOP_AGENT");
					break;
                case  Message_ServiceNo_MANAGE_AGENT:   // manage agent
					dwTypeTmp  = cAppThis->m_pbServerPtr->service_no();
					AGENTLOG_DEBUG_V("message : Message_ServiceNo_MANAGE_AGENT");
					break;				
				default:
					AGENTLOG_ERROR_S( invalid message from MS!);
					zmq_send(cAppThis->m_zmq_server,"", 0, 0); // keep client nonblock
				    continue;
			}
		}

        /* send a message to MS begin ... 
		   send immdiately for keeping MS nonblock	
		*/
		ostringstream ostr;
		cAppThis->m_pbServerPtr->SerializeToOstream(&ostr);
        int res = zmq_send(cAppThis->m_zmq_server,ostr.str().c_str(), ostr.str().length(), 0);
        char tmp[10] = {0};
        sprintf(tmp,"%d", res);
        AGENTLOG_INFO_V( "res = " + string(tmp) + "str = " +ostr.str());
        /* send a message to MS end ... */
	
        string strContenTmp = cAppThis->m_pbServerPtr->content();
        tmpEnType = cAppThis->m_pbServerPtr->enptype();
        tmpCoType = cAppThis->m_pbServerPtr->comtype();
        if (tmpEnType == Message_enptypeNo_NO_ENPTYPE)
        {
            isEncrypted = false;
        }
        else
        {
            isEncrypted = true;
        }
        if (tmpCoType == Message_comtypeNo_NO_COMPRESS)
        {
            isCompressed = false;
        }
        else
        {
            isCompressed = true;
        }
        cAppThis->DeCodeData(strContenTmp, isEncrypted, tmpEnType, isCompressed, tmpCoType);

		AGENTLOG_DEBUG_V("receive a message with content: "+strContenTmp);
		string strModuleId, strPolicy;
		if( !strContenTmp.empty() ) {
            
			if(strContenTmp.find(cAppThis->m_agentConfigPtr->chRecordSep) != string::npos) {
		        strModuleId = strContenTmp.substr(0,strContenTmp.find(cAppThis->m_agentConfigPtr->chRecordSep));			
		        strPolicy   = strContenTmp.substr(strContenTmp.find(cAppThis->m_agentConfigPtr->chRecordSep)+1);
			} else {
			    strModuleId = strContenTmp;
			}
		}
		dwModuleIDTmp = atoi(strModuleId.c_str());	
		tag_reques.dwModuleID = dwModuleIDTmp;      // moduleid
		tag_reques.strPolicy  = strPolicy;          // policy
		tag_reques.dwType	  = dwTypeTmp;
		cAppThis->m_mutex[MUTEX_MS_REQ].lock();
		cAppThis->m_listreq.push_back(tag_reques);  // add request
		cAppThis->m_mutex[MUTEX_MS_REQ].unlock();
	}
	return NULL;
}

//解析策略线程
void *CBDScModuleApp::Worker(void *arg) {

	CBDScModuleApp *cAppThis = reinterpret_cast<CBDScModuleApp*>(arg);

    while(true) {
		char chMsgTmpID[10];    // for print moduleid in loginfo 
		char chMsgTmpType[10];  // for print type in loginfo 
	    while(cAppThis->m_listreq.size() > 0) {
			
	        memset(chMsgTmpID,0,sizeof(chMsgTmpID));
	        memset(chMsgTmpType,0,sizeof(chMsgTmpType));

		    Bd_Request_Itor ItorTmp = cAppThis->m_listreq.begin();
			sprintf(chMsgTmpID,"[%d]",ItorTmp->dwModuleID);
			sprintf(chMsgTmpType,"[%d]",ItorTmp->dwType);

			string strTmp = chMsgTmpID;
		    if( ItorTmp->dwModuleID < 0 ) {
                cAppThis->RequestPop(MUTEX_MS_REQ);				
				AGENTLOG_WARN_V("Invalid dwModuleID " + strTmp);
			    continue;
		    }
		    if( ItorTmp->dwType == Message_ServiceNo_START_PLUGIN ) { // start one module
                bool ret = cAppThis->GetBDScModuleMgr()->SetModuleData(ItorTmp->dwModuleID
                                                                       , NULL //&ItorTmp->dataType_T, 暂时不用
                                                                       , &(ItorTmp->strPolicy)
                                                                       , (ItorTmp->strPolicy).length());

                if (!ret) {
                    AGENTLOG_ERROR_V("Set module Data" + strTmp + " failed! ");
                }

                if (!ret || !cAppThis->GetBDScModuleMgr()->StartModule(ItorTmp->dwModuleID)) {
                    AGENTLOG_ERROR_V("Start module" + strTmp + " failed! ");
                }

                cAppThis->RequestPop(MUTEX_MS_REQ);

            } else if (ItorTmp->dwType == Message_ServiceNo_START_TASK) {  // start one task
                bool ret = cAppThis->GetBDScModuleMgr()->StartModuleTask(ItorTmp->dwModuleID
                                                                       , NULL
                                                                       , &(ItorTmp->strPolicy));

                if (!ret) {
                    AGENTLOG_ERROR_V("StartModuleTask" + strTmp + "failed! ");
                }

                cAppThis->RequestPop(MUTEX_MS_REQ);

            } else if (ItorTmp->dwType == Message_ServiceNo_STOP_PLUGIN) {  // stop one module
                if (!cAppThis->GetBDScModuleMgr()->StopModule(ItorTmp->dwModuleID)) {
                    AGENTLOG_ERROR_V("Stop module" + strTmp + " failed! ");
                }

                cAppThis->RequestPop(MUTEX_MS_REQ);

            } else if (ItorTmp->dwType == Message_ServiceNo_START_AGENT) {  // start agent
                if (!cAppThis->GetBDScModuleMgr()->Start()) {
                    AGENTLOG_ERROR_V("Start agent" + strTmp + " failed! ");
                }

                cAppThis->RequestPop(MUTEX_MS_REQ);

            } else if (ItorTmp->dwType == Message_ServiceNo_STOP_AGENT) {   // stop agent
                cAppThis->GetBDScModuleMgr()->Stop();
                cAppThis->RequestPop(MUTEX_MS_REQ);

            } else if (ItorTmp->dwType == Message_ServiceNo_MANAGE_AGENT) { // manage agent
                vector<string> vecPolicy;
                //string strPolicy = ItorTmp->strPolicy;
                string strFsep = cAppThis->m_agentConfigPtr->chFieldSep;
                string_split(ItorTmp->strPolicy,strFsep,vecPolicy);
                unsigned int nPolicy = 8;
                Message_enptypeNo Enptype_Policy = Message_enptypeNo_NO_ENPTYPE;
                Message_comtypeNo Compressed_Policy = Message_comtypeNo_NO_COMPRESS;

                if(ItorTmp->dwModuleID == 1 || ItorTmp->dwModuleID == 2){
                    nPolicy = atoi(vecPolicy[0].c_str());
                    if(nPolicy == 0){
                        AGENTLOG_WARN_V("Invalid cycle number " + ItorTmp->strPolicy);
                        nPolicy = 8;
                    }
                }
                else if(ItorTmp->dwModuleID == 3){
                    if(vecPolicy.size() == 2)
                    {

                        Enptype_Policy = Message_enptypeNo(atoi(vecPolicy[0].c_str()));
                        Compressed_Policy = Message_comtypeNo(atoi(vecPolicy[1].c_str()));
                    }

                }

                switch(ItorTmp->dwModuleID){
                case 1:
                    cAppThis->m_nTimeSleepHeartBeat = nPolicy * 1000000;
                    break;
                case 2:
                    cAppThis->m_nTimeMonitorHeartBeat = nPolicy;
                    break;
                case 3:
                    if(Enptype_Policy == Message_enptypeNo_NO_ENPTYPE)
                    {
                        cAppThis->m_isEncrypted = false;
                    }
                    else
                    {
                        cAppThis->m_isEncrypted = true;
                    }
                    cAppThis->m_nEncryptType = Enptype_Policy;
                    if(Compressed_Policy == Message_comtypeNo_NO_COMPRESS)
                    {
                        cAppThis->m_isCompressed = false;
                    }
                    else
                    {
                        cAppThis->m_isCompressed = true;
                    }
                    cAppThis->m_nCompressType = Compressed_Policy;

                    break;
                default:
                    AGENTLOG_WARN_V("Invalid dwManageID " + string(chMsgTmpID));
                }
                cAppThis->RequestPop(MUTEX_MS_REQ);
            } else {
				AGENTLOG_WARN_V("Invalid dwType " + string(chMsgTmpType));
			    cAppThis->RequestPop(MUTEX_MS_REQ);
			}
	    }
        usleep(cAppThis->m_nTimeSleepHeartBeat/3 + 1000000);
    }
	return NULL;
}

//上报日志发送接口
bool CBDScModuleApp::Sender(const string &str, const PModIntfDataType pDataType) 
{
    string strContent = str;

    EnCodeData(strContent, m_isEncrypted, m_nEncryptType, m_isCompressed, m_nCompressType);

    Message * PB_Ptr = NULL;
	void * ZMQ_Ptr   = NULL;
	PB_Ptr  = m_MessageObj[pDataType->nMainCategory];  // pb obj
	ZMQ_Ptr = m_ZmqObj[pDataType->nDataType];          // zmq obj
	
	int rc=-1;
    if(PB_Ptr != NULL && ZMQ_Ptr != NULL) {
		ostringstream ostr;
        PB_Ptr->set_content(strContent);
        PB_Ptr->set_enptype(m_nEncryptType);
        PB_Ptr->set_comtype(m_nCompressType);
		time_t t = time(NULL);
		PB_Ptr->set_time((unsigned long long)t);
		PB_Ptr->SerializeToOstream(&ostr);
        rc = zmq_send(ZMQ_Ptr,ostr.str().c_str(), ostr.str().length(), 1);

        if(pDataType->nDataType == MODULE_DATA_LOG)
            ++m_nLog;
    }

    // openvas同步数据太多，屏蔽  @20171128
    if (pDataType->nMainCategory != MODULE_DATA_PERFORMANCE_OPENVAS) {
        AGENTLOG_DEBUG_V("send a message: [" + str + "]");
    }

    return rc > 0? true: false;
}

//运行入口
int CBDScModuleApp::RunProcess() {

	AGENTLOG_INFO_S(RunProcess starts...);

	
    int res = 0;
    res = pthread_create(&m_pthreads[0],NULL,HeartBeatSend,this);
    if( res != 0) {
		AGENTLOG_ERROR_S(Start HeartBeatSend thread failed !...);
	    exit(-1);
    }		
    res = pthread_create(&m_pthreads[1],NULL,HeartBeatRecv,this);
	if( res != 0) {
		AGENTLOG_ERROR_S(Start HeartBeatRecv thread failed !...);
		exit(-1);
	}
    res = pthread_create(&m_pthreads[2],NULL,Reader,this);
	if( res != 0) {
		AGENTLOG_ERROR_S(Start Reader thread failed !...);
		exit(-1);
	}	
	res = pthread_create(&m_pthreads[3],NULL,Worker,this);
	if( res != 0) { 
		AGENTLOG_ERROR_S(Start Worker thread failed !...);
		exit(-1);
	}
	
    // thread exit handle...
    for(int i=0; i<sizeof(m_pthreads); i++) {
		pthread_join(m_pthreads[i],NULL);
    }
    return 0;
}

//判断时间差
int CBDScModuleApp::TimeDiff() {
    static time_t t = time(NULL);
	int ndiff = time(NULL) - t;
	if ( ndiff >= m_nTimeMonitorHeartBeat ) {
		t = time(NULL);
	}
    return ndiff;
}

void CBDScModuleApp::EnCodeData(string &strContent, bool isEcpt, Message_enptypeNo eType, bool isCpss, Message_comtypeNo cType)
{
    /*加密*/
    if(isEcpt){
        try{
            strContent = GetCBOptions()->EncodePw(strContent, eType);
        }
        catch(Poco::Exception& exc){
          AGENTLOG_DEBUG_V("EncodePw  error : "+ exc.displayText());
        }

    }

    /*压缩*/
    if(isCpss){
        ostringstream ostr;  //zip data
        if(cType == Message_comtypeNo_COMPRESS_ZLIB) //1:zlib 2:gzip
        {
            DeflatingOutputStream deflater(ostr, DeflatingStreamBuf::STREAM_ZLIB);
            deflater << strContent;
            deflater.close();  //To ensure buffers get flushed before connected stream closed
        } else
        {
            DeflatingOutputStream deflater(ostr, DeflatingStreamBuf::STREAM_GZIP);
            deflater << strContent;
            deflater.close();
        }
        strContent = ostr.str();
    }
}

void CBDScModuleApp::DeCodeData(string &strContent, bool isEcpt, Message_enptypeNo eType, bool isCpss, Message_comtypeNo cType)
{
    /*解压*/
    if(isCpss){
        istringstream istr(strContent);
        InflatingStreamBuf::StreamType type = InflatingStreamBuf::STREAM_ZLIB;
        if(cType != Message_comtypeNo_COMPRESS_ZLIB)
            type = InflatingStreamBuf::STREAM_GZIP;
        InflatingInputStream infalter(istr, type);

        string strTmp;
        while (infalter.good())          // loop while extraction from file is possible
        {
            char c = infalter.get();    // get character from file
            strTmp.append(1, c);
        }
        if(strTmp.size() > 0) strTmp.erase(strTmp.end() - 1);
        strContent = strTmp;
    }

    /*解密*/
    if(isEcpt){
        try{
            strContent = GetCBOptions()->DecodePw(strContent, eType);
        }
        catch(Poco::Exception& exc){
          AGENTLOG_DEBUG_V("DecodePw error : "+ exc.displayText());
        }

    }
}

/************************************CALLBACK FUNC**********************************************/


bool BD_CALLBACK_TYPE ReportLog(UInt16 nLogType,const void * pData,UInt32 dwLength)
{
	return true;
}

bool BD_CALLBACK_TYPE ReportData(Poco::UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength)
{ 
    //string* strtmp = (string *)pData;
	bool bflag = false;

    /*int typeNum = 0;
    switch (pDataType->nDataType)
    {
        case MODULE_DATA_LOG:
            typeNum = 1;
            break;
        case MODULE_DATA_PERFORMANCE:
            typeNum = 1 << 1;
            break;
        case MODULE_DATA_SERVICE:
            typeNum = 1 << 2;
            break;
        case MODULE_DATA_FLOW:
            typeNum = 1 << 3;
            break;
        case MODULE_DATA_NETFLOW:
            typeNum = 1 << 4;
            break;
        default:
            break;
    }*/

    bflag = CBDScModuleApp::inistance()->Sender(*(string *)pData, pDataType);

	return bflag;
}

const void * BD_CALLBACK_TYPE FetchSysPara(UInt16 nParaType,UInt32& dwRetLen)
{
	return NULL;
}


const void * BD_CALLBACK_TYPE FetchData(Poco::UInt32 dwModuleId,const PModIntfDataType pDataType,Poco::UInt32& dwRetLen)
{
	return NULL;
}

























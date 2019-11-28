
#include "NetflowMgr.h"
#include "utils/bdstring.h"
#include <iostream>
#include "utils/bd_common.h"
#include <algorithm>

//#include "GeoIP.h"
//#include "GeoIPCity.h"

using namespace Poco;
using namespace std;

// #define DATAFILE "/root/SOC/soc5.1/lib_third/GeoIP/data/GeoLiteCity.dat"

CNetflowMgr::CNetflowMgr(const string &strConfigName)
{
	m_strConfigName = strConfigName;
    memset(&m_NetflowConfig,0,sizeof(m_NetflowConfig));
	
	m_pFuncReportData = NULL;
	m_pFuncFetchData  = NULL;
	m_bIsRunning      = false;
	m_bUdpServerState = false;
	pServer           = NULL;

	m_vecPolicy.clear();
}

CNetflowMgr::~CNetflowMgr(void)
{
	m_vecPolicy.clear();
}

//初始化数据，先于Start被调用
bool CNetflowMgr::Init(void)
{
	if(access(m_strConfigName.c_str(), F_OK) != 0) {
        return false;
	}    

    if(!LoadConfig()) {
		return false;
	}
    if( strlen(m_NetflowConfig.chLog4File) > 0 ) {
        if(access(m_NetflowConfig.chLog4File, F_OK) != 0) {
            return false;
	    }
		else {
			 try { 
                log4cxx::PropertyConfigurator::configure(m_NetflowConfig.chLog4File);
	         } catch (Exception &) { 
			     return false;
	         }
		}
    }
	else {
        return false;
	}
	

	

	/*初始化geoip数据库*/
	/*
    gi = GeoIP_open(m_NetflowConfig.Geoipdatafile, GEOIP_MEMORY_CACHE);

	  if (gi == NULL) 
	{
        fprintf(stderr, "Error opening GEOIP database file\n");
        NETFLOW_INFO_S(<=========Error opening GEOIP database file=========>);
        return false;
	}
	*/
    NETFLOW_INFO_S( CNetflowMgr::Init() successed...);
		printConfig();
    //Start();


    return true;
	
}

//检查模块是否处于运行状态
bool CNetflowMgr::IsRunning(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	return m_bIsRunning;
}

UInt32 CNetflowMgr::GetModuleId(void)
{
    return m_NetflowConfig.nModuleId;
}

UInt16 CNetflowMgr::GetModuleVersion(void)
{
    return m_NetflowConfig.wModuleVersion;
}

string CNetflowMgr::GetModuleName(void)
{
    return m_NetflowConfig.chModuleName;
}

//开始（下发)任务
bool CNetflowMgr::StartTask(const PModIntfDataType pDataType,const void * pData)
{
	return true;
}

//停止（取消）任务
bool CNetflowMgr::StopTask(const PModIntfDataType pDataType,const void * pData)
{
    return true;	
}

bool CNetflowMgr::LoadConfig(void)
{	
	AutoPtr<IniFileConfiguration> pConf = NULL;
	try {
		pConf = new IniFileConfiguration(m_strConfigName);
	} catch (NotFoundException& exc ) {
        cerr << "netflow LoadConfig IniFileConfiguration:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
		cerr << "LoadConfig IniFileConfiguration:" << exc.displayText() << endl;	
		return false;
	} catch (Exception& exc) {
        cerr << "netflow LoadConfig:" << exc.displayText() << endl;
		return false;
	}

	try {
        GET_CONF_ITEM_CHAR(log4configfile,m_NetflowConfig.chLog4File,100);
        GET_CONF_ITEM_INT(moduleid,m_NetflowConfig.nModuleId);
        GET_CONF_ITEM_CHAR(modulename,m_NetflowConfig.chModuleName,20);
        GET_CONF_ITEM_INT(moduleversion,m_NetflowConfig.wModuleVersion);
        GET_CONF_ITEM_INT(netflowlistenport,m_NetflowConfig.nListenPort);
        GET_CONF_ITEM_CHAR(record_separator,m_NetflowConfig.chRecordSep,2);
        GET_CONF_ITEM_CHAR(field_separator,m_NetflowConfig.chFieldSep,2);
        GET_CONF_ITEM_INT(listcontrol,m_NetflowConfig.nListCtrl);
        GET_CONF_ITEM_INT(breakcontrol,m_NetflowConfig.nBreakCtrl);
        GET_CONF_ITEM_INT(sleeptimems,m_NetflowConfig.nSleepTimeMs);
        GET_CONF_ITEM_CHAR(Geoipdatafile,m_NetflowConfig.Geoipdatafile,100);

	}
	catch (NotFoundException& exc ) {
        cerr << "netflow LoadConfig:" << exc.displayText() << endl;
		return false;
	} catch (SyntaxException& exc) {
        cerr << "netflow LoadConfig:" << exc.displayText() << endl;
		return false;
	}

    return true;
}

void CNetflowMgr::printConfig() {
    NETFLOW_INFO_S(<=========NETFLOW configure information=========>);
    NETFLOW_INFO_V(" log4configfile=>   ["+string(m_NetflowConfig.chLog4File)+string("]"));
	char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",m_NetflowConfig.nModuleId);
    NETFLOW_INFO_V(" moduleid=>         ["+string(chModuleId)+string("]"));
    NETFLOW_INFO_V(" modulename=>       ["+string(m_NetflowConfig.chModuleName)+string("]"));
	char chVersion[10] = {0};
    sprintf(chVersion,"%d",m_NetflowConfig.wModuleVersion);
    NETFLOW_INFO_V(" wModuleVersion=>   ["+string(chVersion)+string("]"));
	char chPort[6] = {0};
    sprintf(chPort,"%d",m_NetflowConfig.nListenPort);
    NETFLOW_INFO_V(" netflowlistenport=> ["+string(chPort)+string("]"));
    NETFLOW_INFO_V(" record_separator=> ["+string(m_NetflowConfig.chRecordSep)+string("]"));
    NETFLOW_INFO_V(" field_separator=>  ["+string(m_NetflowConfig.chFieldSep)+string("]"));
    NETFLOW_INFO_S(<=========NETFLOW configure information=========>);
}

bool CNetflowMgr::Load(void)
{
	return true;
}

//调用方下发数据
bool CNetflowMgr::SetData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    string strPolicy = *(string *)pData;
	m_vecPolicy.clear();
    m_vecfilter_srcip.clear();
    m_vecfilter_dstip.clear();
    m_vecfilter_prot.clear();

    mask_str.clear();
    prot_str.clear();
    prot7_str.clear();

    string str = m_NetflowConfig.chRecordSep;
    string_split(strPolicy,str,m_vecPolicy);
    if(m_vecPolicy.size() == 3) {
    str = m_NetflowConfig.chFieldSep;
    string_split(m_vecPolicy[0],str,m_vecfilter_srcip);//过滤源ip
    string_split(m_vecPolicy[1],str,m_vecfilter_dstip);//过滤目的ip
    string_split(m_vecPolicy[2],str,m_vecfilter_prot);//过滤协议
    }

    vector<string>::iterator iter = m_vecfilter_srcip.begin();
    NETFLOW_INFO_S(<=========NETFLOW POlOCY=========>);
    for(;iter!= m_vecfilter_srcip.end();iter++)
        NETFLOW_INFO_V(" NETFLOW POlOCY =>" + *iter);
    NETFLOW_INFO_S(<=========NETFLOW POlOCY=========>);
    iter = m_vecfilter_dstip.begin();
    NETFLOW_INFO_S(<=========NETFLOW POlOCY=========>);
    for(;iter!= m_vecfilter_dstip.end();iter++)
        NETFLOW_INFO_V(" NETFLOW POlOCY =>" + *iter);
    NETFLOW_INFO_S(<=========NETFLOW POlOCY=========>);
    iter = m_vecfilter_prot.begin();
    NETFLOW_INFO_S(<=========NETFLOW POlOCY=========>);
    for(;iter!= m_vecfilter_prot.end();iter++)
        NETFLOW_INFO_V(" NETFLOW POlOCY =>" + *iter);
    NETFLOW_INFO_S(<=========NETFLOW POlOCY=========>);
	
    /*CIDR 转成 点分十进制*/
    mask_str.insert(make_pair(1,"128.0.0.0"));
    mask_str.insert(make_pair(2,"192.0.0.0"));
    mask_str.insert(make_pair(3,"224.0.0.0"));
    mask_str.insert(make_pair(4,"240.0.0.0"));
    mask_str.insert(make_pair(5,"248.0.0.0"));
    mask_str.insert(make_pair(6,"252.0.0.0"));
    mask_str.insert(make_pair(7,"254.0.0.0"));
    mask_str.insert(make_pair(8,"255.0.0.0"));
    mask_str.insert(make_pair(9,"255.128.0.0"));
    mask_str.insert(make_pair(10,"255.192.0.0"));
    mask_str.insert(make_pair(11,"255.224.0.0"));
    mask_str.insert(make_pair(12,"255.240.0.0"));
    mask_str.insert(make_pair(13,"255.248.0.0"));
    mask_str.insert(make_pair(14,"255.252.0.0"));
    mask_str.insert(make_pair(15,"255.254.0.0"));
    mask_str.insert(make_pair(16,"255.255.0.0"));
    mask_str.insert(make_pair(17,"255.255.128.0"));
    mask_str.insert(make_pair(18,"255.255.192.0"));
    mask_str.insert(make_pair(19,"255.255.224.0"));
    mask_str.insert(make_pair(20,"255.255.240.0"));
    mask_str.insert(make_pair(21,"255.255.248.0"));
    mask_str.insert(make_pair(22,"255.255.252.0"));
    mask_str.insert(make_pair(23,"255.255.254.0"));
    mask_str.insert(make_pair(24,"255.255.255.0"));
    mask_str.insert(make_pair(25,"255.255.255.128"));
    mask_str.insert(make_pair(26,"255.255.255.192"));
    mask_str.insert(make_pair(27,"255.255.255.224"));
    mask_str.insert(make_pair(28,"255.255.255.240"));
    mask_str.insert(make_pair(29,"255.255.255.248"));
    mask_str.insert(make_pair(30,"255.255.255.252"));
    mask_str.insert(make_pair(31,"255.255.255.254"));
    mask_str.insert(make_pair(32,"255.255.255.255"));

    /*协议 号 转成 协议名称*/
    prot_str.insert(make_pair(0,"HOPOPT"));
    prot_str.insert(make_pair(1,"ICMP"));
    prot_str.insert(make_pair(2,"IGMP"));
    prot_str.insert(make_pair(3,"GGP"));
    prot_str.insert(make_pair(4,"IPv4"));
    prot_str.insert(make_pair(5,"ST"));
    prot_str.insert(make_pair(6,"TCP"));
    prot_str.insert(make_pair(7,"CBT"));
    prot_str.insert(make_pair(8,"EGP"));
    prot_str.insert(make_pair(9,"IGP"));
    prot_str.insert(make_pair(10,"BBN-RCC-MON"));
    prot_str.insert(make_pair(11,"NVP-II"));
    prot_str.insert(make_pair(12,"PUP"));
    prot_str.insert(make_pair(13,"ARGUS"));
    prot_str.insert(make_pair(14,"EMCON"));
    prot_str.insert(make_pair(15,"XNET"));
    prot_str.insert(make_pair(16,"CHAOS"));
    prot_str.insert(make_pair(17,"UDP"));
    prot_str.insert(make_pair(18,"MUX"));
    prot_str.insert(make_pair(19,"DCN-MEAS"));
    prot_str.insert(make_pair(20,"HMP"));
    prot_str.insert(make_pair(21,"PRM"));
    prot_str.insert(make_pair(22,"XNS-IDP"));
    prot_str.insert(make_pair(23,"TRUNK-1"));
    prot_str.insert(make_pair(24,"TRUNK-2"));
    prot_str.insert(make_pair(25,"LEAF-1"));
    prot_str.insert(make_pair(26,"LEAF-2"));
    prot_str.insert(make_pair(27,"RDP"));
    prot_str.insert(make_pair(28,"IRTP"));
    prot_str.insert(make_pair(29,"ISO-TP4"));
    prot_str.insert(make_pair(30,"NETBLT"));
    prot_str.insert(make_pair(31,"MFE-NSP"));
    prot_str.insert(make_pair(32,"MERIT-INP"));
    prot_str.insert(make_pair(33,"DCCP"));
    prot_str.insert(make_pair(34,"3PC"));
    prot_str.insert(make_pair(35,"IDPR"));
    prot_str.insert(make_pair(36,"XTP"));
    prot_str.insert(make_pair(37,"DDP"));
    prot_str.insert(make_pair(38,"IDPR-CMTP"));
    prot_str.insert(make_pair(39,"TP++"));
    prot_str.insert(make_pair(40,"IL"));
    prot_str.insert(make_pair(41,"IPv6"));
    prot_str.insert(make_pair(42,"SDRP"));
    prot_str.insert(make_pair(43,"IPv6-Route"));
    prot_str.insert(make_pair(44,"IPv6-Frag"));
    prot_str.insert(make_pair(45,"IDRP"));
    prot_str.insert(make_pair(46,"RSVP"));
    prot_str.insert(make_pair(47,"GRE"));
    prot_str.insert(make_pair(48,"DSR"));
    prot_str.insert(make_pair(49,"BNA"));
    prot_str.insert(make_pair(50,"ESP"));
    prot_str.insert(make_pair(51,"AH"));
    prot_str.insert(make_pair(52,"I-NLSP"));
    prot_str.insert(make_pair(53,"SWIPE"));
    prot_str.insert(make_pair(54,"NARP"));
    prot_str.insert(make_pair(55,"MOBILE"));
    prot_str.insert(make_pair(56,"TLSP"));
    prot_str.insert(make_pair(57,"SKIP"));
    prot_str.insert(make_pair(58,"IPv6-ICMP"));
    prot_str.insert(make_pair(59,"IPv6-NoNxt"));
    prot_str.insert(make_pair(60,"IPv6-Opts"));
    prot_str.insert(make_pair(62,"CFTP"));
    prot_str.insert(make_pair(64,"SAT-EXPAK"));
    prot_str.insert(make_pair(65,"KRYPTOLAN"));
    prot_str.insert(make_pair(66,"RVD"));
    prot_str.insert(make_pair(67,"IPPC"));
    prot_str.insert(make_pair(69,"SAT-MON"));
    prot_str.insert(make_pair(70,"VISA"));
    prot_str.insert(make_pair(71,"IPCV"));
    prot_str.insert(make_pair(72,"CPNX"));
    prot_str.insert(make_pair(73,"CPHB"));
    prot_str.insert(make_pair(74,"WSN"));
    prot_str.insert(make_pair(75,"PVP"));
    prot_str.insert(make_pair(76,"BR-SAT-MON"));
    prot_str.insert(make_pair(77,"SUN-ND"));
    prot_str.insert(make_pair(78,"WB-MON"));
    prot_str.insert(make_pair(79,"WB-EXPAK"));
    prot_str.insert(make_pair(80,"ISO-IP"));
    prot_str.insert(make_pair(81,"VMTP"));
    prot_str.insert(make_pair(82,"SECURE-VMTP"));
    prot_str.insert(make_pair(83,"VINES"));
    prot_str.insert(make_pair(84,"TTP"));
    prot_str.insert(make_pair(84,"IPTM"));
    prot_str.insert(make_pair(85,"NSFNET-IGP"));
    prot_str.insert(make_pair(86,"DGP"));
    prot_str.insert(make_pair(87,"TCF"));
    prot_str.insert(make_pair(88,"EIGRP"));
    prot_str.insert(make_pair(89,"OSPFIGP"));
    prot_str.insert(make_pair(90,"Sprite-RPC"));
    prot_str.insert(make_pair(91,"LARP"));
    prot_str.insert(make_pair(92,"MTP"));
    prot_str.insert(make_pair(93,"AX.25"));
    prot_str.insert(make_pair(94,"IPIP"));
    prot_str.insert(make_pair(95,"MICP"));
    prot_str.insert(make_pair(96,"SCC-SP"));
    prot_str.insert(make_pair(97,"ETHERIP"));
    prot_str.insert(make_pair(98,"ENCAP"));
    prot_str.insert(make_pair(100,"GMTP"));
    prot_str.insert(make_pair(101,"IFMP"));
    prot_str.insert(make_pair(102,"PNNI"));
    prot_str.insert(make_pair(103,"PIM"));
    prot_str.insert(make_pair(104,"ARIS"));
    prot_str.insert(make_pair(105,"SCPS"));
    prot_str.insert(make_pair(106,"QNX"));
    prot_str.insert(make_pair(107,"A/N"));
    prot_str.insert(make_pair(108,"IPComp"));
    prot_str.insert(make_pair(109,"SNP"));
    prot_str.insert(make_pair(110,"Compaq-Peer"));
    prot_str.insert(make_pair(111,"IPX-in-IP"));
    prot_str.insert(make_pair(112,"VRRP"));
    prot_str.insert(make_pair(113,"PGM"));
    prot_str.insert(make_pair(115,"L2TP"));
    prot_str.insert(make_pair(116,"DDX"));
    prot_str.insert(make_pair(117,"IATP"));
    prot_str.insert(make_pair(118,"STP"));
    prot_str.insert(make_pair(119,"SRP"));
    prot_str.insert(make_pair(120,"UTI"));
    prot_str.insert(make_pair(121,"SMP"));
    prot_str.insert(make_pair(122,"SM"));
    prot_str.insert(make_pair(123,"PTP"));
    prot_str.insert(make_pair(124,"ISIS over IPv4"));
    prot_str.insert(make_pair(125,"FIRE"));
    prot_str.insert(make_pair(126,"CRTP"));
    prot_str.insert(make_pair(127,"CRUDP"));
    prot_str.insert(make_pair(128,"SSCOPMCE"));
    prot_str.insert(make_pair(129,"IPLT"));
    prot_str.insert(make_pair(130,"SPS"));
    prot_str.insert(make_pair(131,"PIPE"));
    prot_str.insert(make_pair(132,"SCTP"));
    prot_str.insert(make_pair(133,"FC"));
    prot_str.insert(make_pair(134,"RSVP-E2E-IGNORE"));
    prot_str.insert(make_pair(135,"Mobility Header"));
    prot_str.insert(make_pair(136,"UDPLite"));
    prot_str.insert(make_pair(137,"MPLS-in-IP"));
    prot_str.insert(make_pair(138,"manet"));
    prot_str.insert(make_pair(139,"HIP"));
    prot_str.insert(make_pair(140,"Shim6"));
    prot_str.insert(make_pair(141,"WESP"));
    prot_str.insert(make_pair(142,"ROHC"));
    prot_str.insert(make_pair(255,"Reserved"));


    /* 默认端口 对应 服务*/
    prot7_str.insert(make_pair(1,"tcpmux"));
    prot7_str.insert(make_pair(5,"rje"));
    prot7_str.insert(make_pair(7,"echo"));
    prot7_str.insert(make_pair(9,"discard"));
    prot7_str.insert(make_pair(11,"systat"));
    prot7_str.insert(make_pair(13,"daytime"));
    prot7_str.insert(make_pair(17,"qotd"));
    prot7_str.insert(make_pair(18,"msp"));
    prot7_str.insert(make_pair(19,"chargen"));
    prot7_str.insert(make_pair(20,"ftp-data"));
    prot7_str.insert(make_pair(21,"ftp"));
    prot7_str.insert(make_pair(22,"ssh"));
    prot7_str.insert(make_pair(23,"telnet"));
    prot7_str.insert(make_pair(25,"smtp"));
    prot7_str.insert(make_pair(37,"time"));
    prot7_str.insert(make_pair(39,"rlp"));
    prot7_str.insert(make_pair(42,"nameserver"));
    prot7_str.insert(make_pair(43,"nicname"));
    prot7_str.insert(make_pair(49,"tacacs"));
    prot7_str.insert(make_pair(50,"re-mail-ck"));
    prot7_str.insert(make_pair(53,"domain"));
    prot7_str.insert(make_pair(63,"whois++"));
    prot7_str.insert(make_pair(67,"bootps"));
    prot7_str.insert(make_pair(68,"bootpc"));
    prot7_str.insert(make_pair(69,"tftp"));
    prot7_str.insert(make_pair(70,"gopher"));
    prot7_str.insert(make_pair(71,"netrjs-1"));
    prot7_str.insert(make_pair(72,"netrjs-2"));
    prot7_str.insert(make_pair(73,"netrjs-3"));
    prot7_str.insert(make_pair(73,"netrjs-4"));
    prot7_str.insert(make_pair(79,"finger"));
    prot7_str.insert(make_pair(80,"http"));
    prot7_str.insert(make_pair(88,"kerberos"));
    prot7_str.insert(make_pair(95,"supdup"));
    prot7_str.insert(make_pair(101,"hostname"));
    prot7_str.insert(make_pair(102,"iso-tsap"));
    prot7_str.insert(make_pair(105,"csnet-ns"));
    prot7_str.insert(make_pair(107,"rtelnet"));
    prot7_str.insert(make_pair(109,"pop2"));
    prot7_str.insert(make_pair(110,"pop3"));
    prot7_str.insert(make_pair(111,"sunrpc"));
    prot7_str.insert(make_pair(113,"auth"));
    prot7_str.insert(make_pair(115,"sftp"));
    prot7_str.insert(make_pair(117,"uucp-path"));
    prot7_str.insert(make_pair(119,"nntp"));
    prot7_str.insert(make_pair(123,"ntp"));
    prot7_str.insert(make_pair(137,"netbios-ns"));
    prot7_str.insert(make_pair(138,"netbios-dgm"));
    prot7_str.insert(make_pair(139,"netbios-ssn"));
    prot7_str.insert(make_pair(143,"imap"));
    prot7_str.insert(make_pair(161,"snmp"));
    prot7_str.insert(make_pair(162,"snmptrap"));
    prot7_str.insert(make_pair(163,"cmip-man"));
    prot7_str.insert(make_pair(164,"cmip-agent"));
    prot7_str.insert(make_pair(174,"mailq"));
    prot7_str.insert(make_pair(177,"xdmcp"));
    prot7_str.insert(make_pair(178,"nextstep"));
    prot7_str.insert(make_pair(179,"bgp"));
    prot7_str.insert(make_pair(191,"prospero"));
    prot7_str.insert(make_pair(194,"irc"));
    prot7_str.insert(make_pair(199,"smux"));
    prot7_str.insert(make_pair(201,"at-rtmp"));
    prot7_str.insert(make_pair(202,"at-nbp"));
    prot7_str.insert(make_pair(204,"at-echo"));
    prot7_str.insert(make_pair(206,"at-zis"));
    prot7_str.insert(make_pair(209,"qmtp"));
    prot7_str.insert(make_pair(210,"z39.50"));
    prot7_str.insert(make_pair(213,"ipx"));
    prot7_str.insert(make_pair(220,"imap3"));
    prot7_str.insert(make_pair(245,"link"));
    prot7_str.insert(make_pair(347,"fatserv"));
    prot7_str.insert(make_pair(363,"rsvp_tunnel"));
    prot7_str.insert(make_pair(369,"rpc2portmap"));
    prot7_str.insert(make_pair(370,"codaauth2"));
    prot7_str.insert(make_pair(372,"ulistproc"));
    prot7_str.insert(make_pair(389,"ldap"));
    prot7_str.insert(make_pair(427,"svrloc"));
    prot7_str.insert(make_pair(434,"mobileip-agent"));
    prot7_str.insert(make_pair(435,"mobilip-mn"));
    prot7_str.insert(make_pair(443,"https"));
    prot7_str.insert(make_pair(444,"snpp"));
    prot7_str.insert(make_pair(445,"microsoft-ds"));
    prot7_str.insert(make_pair(464,"kpasswd"));
    prot7_str.insert(make_pair(468,"photuris"));
    prot7_str.insert(make_pair(487,"saft"));
    prot7_str.insert(make_pair(488,"gss-http"));
    prot7_str.insert(make_pair(496,"pim-rp-disc"));
    prot7_str.insert(make_pair(500,"isakmp"));
    prot7_str.insert(make_pair(535,"iiop"));
    prot7_str.insert(make_pair(538,"gdomap"));
    prot7_str.insert(make_pair(546,"dhcpv6-client"));
    prot7_str.insert(make_pair(547,"dhcpv6-server"));
    prot7_str.insert(make_pair(554,"rtsp"));
    prot7_str.insert(make_pair(563,"nntps"));
    prot7_str.insert(make_pair(565,"whoami"));
    prot7_str.insert(make_pair(587,"submission"));
    prot7_str.insert(make_pair(610,"npmp-local"));
    prot7_str.insert(make_pair(611,"npmp-gui"));
    prot7_str.insert(make_pair(612,"hmmp-ind"));
    prot7_str.insert(make_pair(631,"ipp"));
    prot7_str.insert(make_pair(636,"ldaps"));
    prot7_str.insert(make_pair(674,"acap"));
    prot7_str.insert(make_pair(694,"ha-cluster"));
    prot7_str.insert(make_pair(749,"kerberos-adm"));
    prot7_str.insert(make_pair(750,"kerberos-iv"));
    prot7_str.insert(make_pair(765,"webster"));
    prot7_str.insert(make_pair(767,"phonebook"));
    prot7_str.insert(make_pair(873,"rsync"));
    prot7_str.insert(make_pair(992,"telnets"));
    prot7_str.insert(make_pair(993,"imaps"));
    prot7_str.insert(make_pair(994,"ircs"));
    prot7_str.insert(make_pair(995,"pop3s"));
    prot7_str.insert(make_pair(1080,"socks"));
    prot7_str.insert(make_pair(1236,"bvcontrol"));
    prot7_str.insert(make_pair(1300,"h323hostcallsc"));
    prot7_str.insert(make_pair(1433,"ms-sql-s"));
    prot7_str.insert(make_pair(1434,"ms-sql-m"));
    prot7_str.insert(make_pair(1494,"ica"));
    prot7_str.insert(make_pair(1512,"wins"));
    prot7_str.insert(make_pair(1524,"ingreslock"));
    prot7_str.insert(make_pair(1525,"prospero-np"));
    prot7_str.insert(make_pair(1645,"datametrics"));
    prot7_str.insert(make_pair(1646,"sa-msg-port"));
    prot7_str.insert(make_pair(1649,"kermit"));
    prot7_str.insert(make_pair(1701,"l2tp"));
    prot7_str.insert(make_pair(1718,"h323gatedisc"));
    prot7_str.insert(make_pair(1719,"h323gatestat"));
    prot7_str.insert(make_pair(1720,"h323hostcall"));
    prot7_str.insert(make_pair(1758,"tftp-mcast"));
    prot7_str.insert(make_pair(1759,"mtftp"));
    prot7_str.insert(make_pair(1789,"hello"));
    prot7_str.insert(make_pair(1812,"radius"));
    prot7_str.insert(make_pair(1813,"radius-acct"));
    prot7_str.insert(make_pair(1911,"mtp"));
    prot7_str.insert(make_pair(1985,"hsrp"));
    prot7_str.insert(make_pair(1986,"licensedaemon"));
    prot7_str.insert(make_pair(1997,"gdp-port"));
    prot7_str.insert(make_pair(2049,"nfs"));
    prot7_str.insert(make_pair(2102,"zephyr-srv"));
    prot7_str.insert(make_pair(2103,"zephyr-clt"));
    prot7_str.insert(make_pair(2104,"zephyr-hm"));
    prot7_str.insert(make_pair(2401,"cvspserver"));
    prot7_str.insert(make_pair(2430,"venus"));
    prot7_str.insert(make_pair(2430,"venus"));
    prot7_str.insert(make_pair(2431,"venus-se"));
    prot7_str.insert(make_pair(2431,"venus-se"));
    prot7_str.insert(make_pair(2432,"codasrv"));
    prot7_str.insert(make_pair(2433,"codasrv-se"));
    prot7_str.insert(make_pair(2433,"codasrv-se"));
    prot7_str.insert(make_pair(2600,"hpstgmgr"));
    prot7_str.insert(make_pair(2601,"discp-client"));
    prot7_str.insert(make_pair(2602,"discp-server"));
    prot7_str.insert(make_pair(2603,"servicemeter"));
    prot7_str.insert(make_pair(2604,"nsc-ccs"));
    prot7_str.insert(make_pair(2605,"nsc-posa"));
    prot7_str.insert(make_pair(2606,"netmon"));
    prot7_str.insert(make_pair(2809,"corbaloc"));
    prot7_str.insert(make_pair(3130,"icpv2"));
    prot7_str.insert(make_pair(3306,"mysql"));
    prot7_str.insert(make_pair(3346,"trnsprntproxy"));
    prot7_str.insert(make_pair(4011,"pxe"));
    prot7_str.insert(make_pair(4321,"rwhois"));
    prot7_str.insert(make_pair(4444,"krb524"));
    prot7_str.insert(make_pair(5002,"rfe"));
    prot7_str.insert(make_pair(5308,"cfengine"));
    prot7_str.insert(make_pair(5999,"cvsup"));
    prot7_str.insert(make_pair(6000,"x11"));
    prot7_str.insert(make_pair(7000,"afs3-fileserver"));
    prot7_str.insert(make_pair(7001,"afs3-callback"));
    prot7_str.insert(make_pair(7002,"afs3-prserver"));
    prot7_str.insert(make_pair(7003,"afs3-vlserver"));
    prot7_str.insert(make_pair(7004,"afs3-kaserver"));
    prot7_str.insert(make_pair(7005,"afs3-volser"));
    prot7_str.insert(make_pair(7006,"afs3-errors"));
    prot7_str.insert(make_pair(7007,"afs3-bos"));
    prot7_str.insert(make_pair(7008,"afs3-update"));
    prot7_str.insert(make_pair(7009,"afs3-rmtsys"));
    prot7_str.insert(make_pair(9876,"sd"));
    prot7_str.insert(make_pair(10080,"amanda"));
    prot7_str.insert(make_pair(11371,"pgpkeyserver"));
    prot7_str.insert(make_pair(11720,"h323callsigalt"));
    prot7_str.insert(make_pair(13720,"bprd"));
    prot7_str.insert(make_pair(13721,"bpdbm"));
    prot7_str.insert(make_pair(13722,"bpjava-msvc"));
    prot7_str.insert(make_pair(13724,"vnetd"));
    prot7_str.insert(make_pair(13782,"bpcd"));
    prot7_str.insert(make_pair(13783,"vopied"));
    prot7_str.insert(make_pair(22273,"wnn6"));
    prot7_str.insert(make_pair(26000,"quake"));
    prot7_str.insert(make_pair(26208,"wnn6-ds"));
    prot7_str.insert(make_pair(33434,"traceroute"));
    prot7_str.insert(make_pair(15,"netstat"));
    prot7_str.insert(make_pair(98,"linuxconf"));
    prot7_str.insert(make_pair(106,"poppassd"));
    prot7_str.insert(make_pair(465,"smtps"));
    prot7_str.insert(make_pair(616,"gii"));
    prot7_str.insert(make_pair(808,"omirr"));
    prot7_str.insert(make_pair(871,"supfileserv"));
    prot7_str.insert(make_pair(901,"swat"));
    prot7_str.insert(make_pair(953,"rndc"));
    prot7_str.insert(make_pair(1127,"sufiledbg"));
    prot7_str.insert(make_pair(1178,"skkserv"));
    prot7_str.insert(make_pair(1313,"xtel"));
    prot7_str.insert(make_pair(1529,"support"));
    prot7_str.insert(make_pair(2003,"cfinger"));
    prot7_str.insert(make_pair(2150,"ninstall"));
    prot7_str.insert(make_pair(2988,"afbackup"));
    prot7_str.insert(make_pair(3128,"squid"));
    prot7_str.insert(make_pair(3455,"prsvp"));
    prot7_str.insert(make_pair(5432,"postgres"));
    prot7_str.insert(make_pair(4557,"fax"));
    prot7_str.insert(make_pair(4559,"hylafax"));
    prot7_str.insert(make_pair(5232,"sgi-dgl"));
    prot7_str.insert(make_pair(5354,"noclog"));
    prot7_str.insert(make_pair(5355,"hostmon"));
    prot7_str.insert(make_pair(5680,"canna"));
    prot7_str.insert(make_pair(6010,"x11-ssh-offset"));
    prot7_str.insert(make_pair(6667,"ircd"));
    prot7_str.insert(make_pair(7100,"xfs"));
    prot7_str.insert(make_pair(7666,"tircproxy"));
    prot7_str.insert(make_pair(8008,"http-alt"));
    prot7_str.insert(make_pair(8080,"webcache"));
    prot7_str.insert(make_pair(8081,"tproxy"));
    prot7_str.insert(make_pair(9100,"jetdirect"));
    prot7_str.insert(make_pair(9359,"mandelspawn"));
    prot7_str.insert(make_pair(10081,"kamanda"));
    prot7_str.insert(make_pair(10082,"amandaidx"));
    prot7_str.insert(make_pair(10083,"amidxtape"));
    prot7_str.insert(make_pair(20011,"isdnlog"));
    prot7_str.insert(make_pair(20012,"vboxd"));
    prot7_str.insert(make_pair(22305,"wnn4_Kr"));
    prot7_str.insert(make_pair(22289,"wnn4_Cn"));
    prot7_str.insert(make_pair(22321,"wnn4_Tw"));
    prot7_str.insert(make_pair(24554,"binkp"));
    prot7_str.insert(make_pair(27374,"asp"));
    prot7_str.insert(make_pair(60177,"tfido"));
    prot7_str.insert(make_pair(60179,"fido"));

	return true;
}

//调用方获取数据
void * CNetflowMgr::GetData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//调用方释放获取到的内存
void CNetflowMgr::FreeData(void * pData)
{
	
}

//set callback for report function
void CNetflowMgr::SetReportData(pFunc_ReportData pCbReport)
{
	m_pFuncReportData = pCbReport; 
}

//set call back for fetch function
void CNetflowMgr::SetFetchData(pFunc_FetchData pCbFetch)
{
	m_pFuncFetchData = pCbFetch;
}

//Report Data
bool CNetflowMgr::ReportData(const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
    bool bRet = false;
	
    if (m_pFuncReportData){
        bRet =  m_pFuncReportData(m_NetflowConfig.nModuleId,pDataType,pData,dwLength);
    }
    return bRet;
}

//Fetch Data
const char* CNetflowMgr::FetchData(const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	return NULL;
}

//启动模块
bool CNetflowMgr::Start(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
	//stop后会导致单例失效
    pServer = netUdpClientService::inistance();
    if( pServer == NULL ) return false;
    pServer->m_netflow_result.clear();
    //pServer->m_inputMutex.unlock();

    if( !m_bIsRunning) {
        pthread_attr_t pa;       //线程属性
        pthread_attr_init(&pa);	//初始化线程属性
        pthread_attr_setdetachstate(&pa, PTHREAD_CREATE_DETACHED);	   //设置线程属性
        if (pthread_create(&p_thread_udp, &pa, OnStartUdpServer, this) == -1) {
            NETFLOW_ERROR_S(Create OnStartUdpServer Thread failed!);
			return false;
        }		
        NETFLOW_INFO_S(Create OnStartUdpServer Thread successed!);
		usleep(50000);// 50毫秒
        if (pthread_create(&p_thread_report, &pa, OnReportHandle, this) != 0) {
            NETFLOW_ERROR_S(Create OnReportHandle Thread failed!);
			return false;
        }
        NETFLOW_INFO_S(Create OnReportHandle Thread successed!);
	}
    usleep(50000);// 50毫秒，启动线程需要一些执行时间
    NETFLOW_INFO_S(CNetflowMgr::Start successed!);
	
	return true;
}

//停止模块
bool CNetflowMgr::Stop(void)
{
	FastMutex::ScopedLock lock(m_ModMutex);
    //停UDP线程,状态由线程自己改变
    pServer->handle_close(pServer->get_handle(), 0);
    // m_bIsRunning状态在OnStartUdpServer中改变
	//ACE_Reactor::instance()->end_event_loop();	
        netUdpClientService::pReactor->end_reactor_event_loop();

    NETFLOW_INFO_S(CNetflowMgr::Stop finished!);
    return true;
}

void *CNetflowMgr::OnFetchHandle(void *arg) {
    return NULL;
}

//开启UDP监听以及I/O事件检测
void* CNetflowMgr::OnStartUdpServer(void *arg)
{
    CNetflowMgr *cThis = reinterpret_cast<CNetflowMgr*>(arg);

	//ACE_Reactor* pReactor = ACE_Reactor::instance();
	//cThis->pServer->reactor(pReactor);
   //NETFLOW_INFO_V("cThis->m_NetflowConfig.nListenPort = " + cThis->m_NetflowConfig.nListenPort);
       netUdpClientService::pReactor = new ACE_Reactor( (ACE_Reactor_Impl *)new ACE_TP_Reactor(), true);
    if( cThis->pServer->open(cThis->m_NetflowConfig.nListenPort) != 0 ) {
		cThis->pServer->handle_close(ACE_INVALID_HANDLE, 0);	
		cThis->m_bIsRunning = true;
        NETFLOW_ERROR_S(OnStartUdpServer end ...);
		return NULL;
	}
    //NETFLOW_INFO_S("cThis->m_NetflowConfig.nListenPort = " + cThis->m_NetflowConfig.nListenPort);
    cThis->m_bUdpServerState = true;//避免端口占用后，监听线程停止，report线程无法停止
	cThis->pServer->pReactor->run_reactor_event_loop(); //阻塞，内部循环
    NETFLOW_INFO_S(run_reactor_event_loop is ended !);
 	sleep(1);
	cThis->pServer->pReactor->close();

	cThis->m_bUdpServerState = false;
	if ( cThis->pServer->pReactor != NULL )
		delete cThis->pServer->pReactor;
	cThis->pServer->pReactor = NULL;


    NETFLOW_INFO_S(OnStartUdpServer end ...);
	return NULL;
}

//获取日志
void *CNetflowMgr::OnReportHandle(void *arg)
{   
    CNetflowMgr *cThis = reinterpret_cast<CNetflowMgr*>(arg);
	while(true) {
		if (cThis->m_bUdpServerState) {  // udp 线程启动后，插件状态才更新为true
        cThis->m_bIsRunning = true;
        }
    bool  bReportCtrl = false;
    /*
	string src_country;
	string src_city;
	double src_latitude;
	char src_lat_temp[1024];
	double src_longitude;
	char src_lon_temp[1024];
	string dst_country;
	string dst_city;
	double dst_latitude;
	char dst_lat_temp[1024];
	double dst_longitude;
	char dst_lon_temp[1024];
    */
	while( cThis->m_bUdpServerState && cThis->m_bIsRunning ) {

        static int num = 0;
		
        if ( cThis->pServer->m_netflow_result.size() >  cThis->m_NetflowConfig.nListCtrl || bReportCtrl) {

            cThis->pServer->m_inputMutex.lock();
            list<netflow_record_result>::iterator iter = cThis->pServer->m_netflow_result.begin();
            vector<string>::iterator filer_iter;
            int nBreakCtrl = 0;
            int filer_flag = 0;//过滤标志，置为1则跳过本条记录
            for(;iter != cThis->pServer->m_netflow_result.end();)
            {
                  filer_flag = 0;
                  //过滤源ip
                  filer_iter =  find( cThis->m_vecfilter_srcip.begin(), cThis->m_vecfilter_srcip.end( ), iter->srcaddr);
                  if ( filer_iter != cThis->m_vecfilter_srcip.end( ) ) //找到
                  {
                     filer_flag = 1;
                  }
                    //过滤目的ip
                  if(filer_flag == 0)
                  {
                      filer_iter =  find( cThis->m_vecfilter_dstip.begin(), cThis->m_vecfilter_dstip.end( ), iter->dstaddr);
                      if ( filer_iter != cThis->m_vecfilter_dstip.end( ) ) //找到
                      {
                         filer_flag = 1;
                      }
                  }



                string prot_cur; //当前信息流3层协议
                if(cThis->prot_str[iter->prot] == ""){
                    prot_cur = bd_int2str(iter->prot);
                }else{
                    prot_cur = cThis->prot_str[iter->prot];
                }

                //过滤信息流3层协议
                if(filer_flag == 0)
                {
                    filer_iter =  find( cThis->m_vecfilter_prot.begin(), cThis->m_vecfilter_prot.end( ), prot_cur);
                    if ( filer_iter != cThis->m_vecfilter_prot.end( ) ) //找到
                    {
                       filer_flag = 1;
                    }

                }


                if(filer_flag == 1)
                {
                    iter = cThis->pServer->m_netflow_result.erase(iter);
                    continue;
                }


                /*获取ip地址地理位置信息*/
                /*
                  GeoIPRecord *src_gir = GeoIP_record_by_name(cThis->gi, iter->srcaddr.c_str());
                  if(src_gir==NULL)
                  {
                      src_city = "";
                      src_country = "";
                      src_latitude = 0;
                      src_longitude = 0;

                  }
                  else
                  {
					if( src_gir->country_name!=NULL)
					{
                      src_country = src_gir->country_name;
					}
					if( src_gir->city!=NULL)
					{
                      src_city = src_gir->city;
					}
                      src_latitude = src_gir->latitude;
                      src_longitude = src_gir->longitude;
                      GeoIPRecord_delete(src_gir);
                  }

                  memset(src_lat_temp,0,sizeof(src_lat_temp));
                  sprintf(src_lat_temp,"%lf", src_latitude);

                  memset(src_lon_temp,0,sizeof(src_lon_temp));
                  sprintf(src_lon_temp,"%lf", src_longitude);

                  GeoIPRecord *dst_gir = GeoIP_record_by_name(cThis->gi, iter->dstaddr.c_str());


                if(dst_gir == NULL)
                {
                    dst_city = "";
                    dst_country = "";
                    dst_latitude = 0;
                    dst_longitude = 0;
                }
                else
                {
					if( dst_gir->country_name!=NULL)
					{
						dst_country = dst_gir->country_name;
					}
					if( dst_gir->city!=NULL)
					{
						dst_city = dst_gir->city;
					}
                    dst_latitude = dst_gir->latitude;
					dst_longitude = dst_gir->longitude;
                    GeoIPRecord_delete(dst_gir);
                }

                memset(dst_lat_temp,0,sizeof(dst_lat_temp));
                sprintf(dst_lat_temp,"%lf", dst_latitude);

                memset(dst_lon_temp,0,sizeof(dst_lon_temp));
                sprintf(dst_lon_temp,"%lf", dst_longitude);
                */
                string strtmp = "netflow";
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "AGENT_IP";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += iter->agent_ip;
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRCIP";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += iter->srcaddr;
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DSTIP";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += iter->dstaddr;
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "NEXTHOP";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += iter->nexthop;
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "ETH_IN";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->input);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "ETH_OUT";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->output);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRCIP_AREA";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->src_as);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DSTIP_AREA";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->dst_as);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRCIP_NETMASK";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += cThis->mask_str[iter->src_mask];
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DSTIP_NETMASK";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += cThis->mask_str[iter->dst_mask];
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "STARTTIME";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->First);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "ENDTIME";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->Last);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRCPORT";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->srcport);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DSTPORT";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->dstport);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "PROTO";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += prot_cur;
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "PROTO7";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                if(cThis->prot7_str[iter->dstport] == ""){
                    strtmp += bd_int2str(iter->dstport);
                }else{
                    strtmp += cThis->prot7_str[iter->dstport];
                }
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "PACKERNUM";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->dPkts);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "BYTESIZE";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->dOctets);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "FLOWNUM";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->flownum);
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "RECORDTIME";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(iter->record_time);

                int flag = loadingstat(iter->srcaddr, iter->dstaddr);

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "FLAG";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += bd_int2str(flag);

                /*
                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRC_COUNTRY";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += src_country;

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRC_CITY";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += src_city;

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRC_LAT";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += string(src_lat_temp);

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "SRC_LON";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += string(src_lon_temp);

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DST_COUNTRY";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += dst_country;

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DST_CITY";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += dst_city;

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DST_LAT";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += string(dst_lat_temp);

                strtmp += cThis->m_NetflowConfig.chRecordSep;
                strtmp += "DST_LON";
                strtmp += cThis->m_NetflowConfig.chFieldSep;
                strtmp += string(dst_lon_temp);
                */

				NETFLOW_DEBUG_V("send data =>" + strtmp);

                modintf_datatype_t temp;
                temp.nDataType     = MODULE_DATA_NETFLOW;
                temp.nMainCategory = MODULE_DATA_FLOW_NETFLOW;
                cThis->m_pFuncReportData(cThis->m_NetflowConfig.nModuleId,&temp,(void *)&strtmp,strtmp.length());
                ++nBreakCtrl;
                iter = cThis->pServer->m_netflow_result.erase(iter);
                if (nBreakCtrl > cThis->m_NetflowConfig.nBreakCtrl) break;
            }
            cThis->pServer->m_inputMutex.unlock();
        }

        if ( num == 0) {
             num = cThis->pServer->m_netflow_result.size();
             bReportCtrl =false;
        } else  if( num == cThis->pServer->m_netflow_result.size()){
            bReportCtrl = true;
        }

        if(cThis->pServer->m_netflow_result.size() < 10000)
            usleep(cThis->m_NetflowConfig.nSleepTimeMs * 1000);
        else
            usleep(10 * 1000);

    }

        if ( !cThis->m_bUdpServerState && cThis->m_bIsRunning) { // dup 线程退出了
                break;
        }
		sleep(1);
    }
        cThis->pServer->Destroy();
        cThis->m_bIsRunning = false;
/*
        if (cThis->gi != NULL) {
                GeoIP_delete(cThis->gi);
        }
		
		*/
        NETFLOW_INFO_S(OnReportHandle end ...);
        return NULL;
}

//base64解码
string CNetflowMgr::fromBase64 (const string &source)
{
  istringstream in(source);
  ostringstream out;
  Base64Decoder b64in(in);
  
  copy(istreambuf_iterator<char>(b64in),
            istreambuf_iterator<char>(),
            ostreambuf_iterator<char>(out));
  
  return out.str();
}


bool CNetflowMgr::isInnerIP (string IP)  //判断是否内网ip
{
    bool flag = false;
    string strA = "10.";
    string strB = "172.";
    string strC = "192.168.";

    if(IP[3] == '.')  //判断是否组播ip
    {
        int itemp = atoi(IP.substr(0,3).c_str());
        if (itemp <= 239 && itemp >= 224)
        {
            flag = true;
        }
    }
    if (IP.compare(0, 3, strA) == 0 || IP.compare(0, 8, strC) == 0)
    {
        flag = true;
    }
    if (IP.compare(0, 4, strB) == 0 && IP[6] == '.')
    {
        int itemp = atoi(IP.substr(4,2).c_str());
        if (itemp <= 31 && itemp >= 16)
        {
            flag = true;
        }
    }
    return flag;
}

int CNetflowMgr::loadingstat(string srcip, string dstip) //流量是判断上传下载
{

    int stat = 3;  //0.上行；1.下行；3.内网间；4.外网间

    if(srcip.length() < 10 || dstip.length() < 10)
    {
        return stat;
    }

    bool srcisInnerflag = isInnerIP(srcip);
    bool dstisInnerflag = isInnerIP(dstip);

    if (srcisInnerflag == false && dstisInnerflag == true) //如果源ip是外网，目的ip是内网，则是下载
    {
        stat = 1;
    }
    else if (srcisInnerflag == true && dstisInnerflag == false) //如果源ip是内网，目的ip是外网，则是上传
    {
        stat = 0;
    }
    else if (srcisInnerflag == true && dstisInnerflag == true) //如果源ip是内网，目的ip是内网，则是内网间
    {
        stat = 3;
    }
    else
    {
        stat = 4;
    }
    return stat;

}

#ifndef _H_COMMON_BDICSP
#define _H_COMMON_BDICSP

#include "Poco/Types.h"

using namespace Poco;
using namespace std;

#if defined(POCO_OS_FAMILY_WINDOWS)
#pragma warning( disable : 4200 )
#endif


#define  AGENTLOG_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("AGENT"), str)
#define  AGENTLOG_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("AGENT"), str)
#define  AGENTLOG_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("AGENT"), str)
#define  AGENTLOG_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("AGENT"), str)

#define  AGENTLOG_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("AGENT"), #str)
#define  AGENTLOG_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("AGENT"), #str)
#define  AGENTLOG_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("AGENT"), #str)
#define  AGENTLOG_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("AGENT"), #str)





#define DEL_PTR(ptr) while(ptr){delete ptr;ptr = NULL;}
#define DEL_ZMQ(zmq) while(zmq){zmq_close(zmq);zmq = NULL;}


/*******************************************************************/


#pragma pack(push,1)
#define BDHIDS_REPLY_FLAG 0x8000
#define IS_REPLY_PKT(pkt) (((pkt)->reqid & BDHIDS_REPLY_FLAG) != 0)

#define BDICSPCP_TAG "BDHSECCP"
#define BDICSPCP_VER 0x0100

typedef struct tagBdIcspCpEx
{
	char pkttag[8];    // BDHSECCP_TAG
	UInt16 version; // BDHSECCP_VER
	UInt16 reqid;
	UInt32 id;
	UInt16 chksum;
	UInt32 datalen;
	char reserved[20];
	char data[0];
} bdicspcp_head_t,*PBdIcspCpHead;

//监管中心后台与代理主机软件请求包协议定义
#define BDICSPCP_CS_REQID_LOGIN  0x0001  //登陆
#define BDICSPCP_CS_REQID_LOGOUT 0x0002  //登出
#define BDICSPCP_CS_REQID_HEARTBEAT 0x0003 //心跳
#define BDICSPCP_CS_REQID_FETCH_HOSTID 0x0004 //获取主机ID
#define BDICSPCP_CS_REQID_REPORT_HOST_REGINFO 0x0005//上报主机注册信息
#define BDICSPCP_CS_REQID_UNINST_HOST 0x0006//卸载主机 
#define BDICSPCP_CS_REQID_DOWN_DEPT 0x0007//下载部门信息
#define BDICSPCP_CS_REQID_DOWN_CLIENT_PARA 0x0008 //下载客户端全局参数
#define BDICSPCP_CS_REQID_DOWN_HOST_BASEINFO 0x0009//下载主机基本信息
#define BDICSPCP_CS_REQID_REPORT_HOST_BASEINFO 0x0010//上报主机基本信息
#define BDICSPCP_CS_REQID_DOWN_HOST_SCPARA 0x0011//下载监控参数
#define BDICSPCP_CS_REQID_DOWN_HOST_SCPOLICY 0X0012 //下载监控策略
#define BDICSPCP_SC_REQID_START_HOST_SCPOLICY 0x0013//启动主机监控策略
#define BDICSPCP_SC_REQID_STOP_HOST_SCPOLICY 0x0014//停止主机监控策略
#define BDICSPCP_SC_REQID_DOWN_ALERT_PARA 0x0015 //下发告警参数
#define BDICSPCP_CS_REQID_REPORT_SCLOG 0x0016 //上报监控日志
#define BDICSPCP_CS_REQID_DOWN_FILE 0x0017//下发文件

//总体响应报文定义
#define BDICSPCP_REPID_STATE  0x8000//响应状态报文
#define BDICSPCP_REPID_RESULT  0x8001//一般响应结果报文（无须返回内容）
#define BDICSPCP_REPID_RESULT_FREE_HOSTID  0x8002//返回空闲主机ID
#define BDICSPCP_REPID_RESULT_SYSAUTHINFO   0x8003//返回系统授权信息

#define BDICSP_MAC_LEN 20    
#define BDICSP_HOSTID_LEN 6
#define BDICSP_IPADDR_LEN 64
#define BDICSP_PW_LEN 20
#define BDICSP_AREACODE_LEN 6
#define BDICSP_AREANAME_LEN 32
#define BDICSP_DEPTNAME_LEN 32
#define BDICSP_USEMAN_LEN 10 //old:32
#define BDICSP_PATH_LEN 4096
#define BDICSP_INTF_LEN 40

#define BDICSP_DEVNO_LEN 40 //设备编号
#define BDICSP_DEVNAME_LEN 80//设备名称
#define BDICSP_DEVPOS_LEN 80//设备位置
#define BDICSP_DEVVID_LEN 4 //设备VID
#define BDICSP_DEVPID_LEN 4 //设备PID
#define BDICSP_DEVUID_LEN 32 //设备PID

#define BDICSP_PSNAME_LEN 256
#define BDICSP_PSPATH_LEN 4096
#define BDICSP_PSPATH_LEN 4096

//连续包标识定义
#define PACK_FLAG_BEGIN 0x01 //起始标识
#define PACK_FLAG_END   0x02 //终止标识


//机器类型
enum ClientMachineType
{
    DESKTOP_MACHINE = 1, //桌面计算机
    PORTABLE_MACHINE = 2,//便携式计算机
    SERVER_MACHINE = 3,  //服务器
    IPC_MACHINE = 4,     //工控机
    ETC_MACHINE = 5     //其它
};
// 主机基本信息

#define BASEINFO_BRANDMODEL_LEN 32
#define BASEINFO_PHYPOS_LEN 32
#define BASEINFO_TELPHONE_LEN 20
#define BASEINFO_NOTES_LEN 40
typedef struct tagHosBaseInfoEx 
{
    char areacode[BDICSP_AREACODE_LEN+1]; //地区编号
    char dept[BDICSP_DEPTNAME_LEN+1];  //部门
	char useman[BDICSP_USEMAN_LEN+1]; //使用人
	UInt8 machinetype;//机器类型
	char brand_model[BASEINFO_BRANDMODEL_LEN+1]; //品牌型号
	char phypos[BASEINFO_PHYPOS_LEN+1];//物理位置
    UInt8 seclevel;//密级
	char telphone[BASEINFO_TELPHONE_LEN+1];//联系电话
	char notes[BASEINFO_NOTES_LEN+1];//备注	
} bdicsp_hostbaseinfo_t;

//主机信息

//主机类型
enum ClientHostType 
{   
    CLIENT = 1,  //客户端
    SERVER = 2   //服务器端
};


#define HOSTINFO_CLIENTVER_LEN 12
#define HOSTINFO_PLATFORM_LEN 80
#define HOSTINFO_LOGINUSER_LEN 32
#define HOSTINFO_HOSTNAME_LEN 64
#define HOSTINFO_HDSN_LEN 80

typedef struct tagHostInfoEx 
{  
	UInt8 hosttype; //客户端类型  
	bdicsp_hostbaseinfo_t host_baseinfo;//主机基本信息
	char ipaddr[BDICSP_IPADDR_LEN+1]; //主机IP地址
	char mac[BDICSP_MAC_LEN+1]; //主机MAC地址
	char hostid[BDICSP_HOSTID_LEN+1]; //主机ID
	char clientver[HOSTINFO_CLIENTVER_LEN+1];//客户端版本
	char platform[HOSTINFO_PLATFORM_LEN+1]; //操作系统平台
	char loginuser[HOSTINFO_LOGINUSER_LEN+1]; //当前系统附录用户
	char hostname[HOSTINFO_HOSTNAME_LEN+1]; //主机名
	char hdsn[HOSTINFO_HDSN_LEN+1];//硬盘序列号
	char authpw[BDICSP_PW_LEN+1];//认证密码 
} bdicsp_hostinfo_t;     

//获取空闲主机ID信息
typedef struct tagFetchFreeHostidEx
{
    char mac[BDICSP_MAC_LEN+1]; //主机MAC地址
    char hdsn[HOSTINFO_HDSN_LEN+1];//硬盘序列号
} bdicsp_freehostid_t;
//部门信息
typedef struct tagDeptMsgEx
{
    UInt32 deptid;
    char deptname[BDICSP_DEPTNAME_LEN+1];
} bdicsp_deptinfo_t;
typedef struct tagDeptMsgListEx
{
	bdicsp_deptinfo_t dept;
	struct tagDeptMsgListEx * next;
} bdicsp_deptinfo_list;

typedef struct tagAreaMsgEx
{
	char areacode[BDICSP_AREACODE_LEN+1];
	char areaname[BDICSP_AREANAME_LEN+1];
} bdicsp_areainfo_t;

//部门列表信息(用于解码）
typedef struct tagDeptListEx
{
    UInt32 pack_flag;//包标志,0x01:起始,0x02终止
    char areacode[BDICSP_AREACODE_LEN+1];
	char areaname[BDICSP_AREANAME_LEN+1];
	bdicsp_deptinfo_list * pDeptInfoList;
} bdicsp_dept_list;


//客户端全局参数
#define GLOBALPARA_CENTERID_LEN 40
#define GLOBALPARA_CENTERNAME_LEN 64

//监控模块定义
enum BDScModuleIdDef
{
	 SCMODULE_NACMON=1, // 非法主机接入监控
	 SCMODULE_USBMON=2,//USB端口配置监控
	 SCMODULE_USBTORMON=3,//移动介质监控
	 SCMODULE_PSNETBHMON=4,//非法连接其他设备监控
	 SCMODULE_EXTRACCONNMON=5,//非法连接外网监控
	 SCMODULE_HDMON=6,//服务器存储单元监控
	 SCMODULE_DBAUDITMON=7,//服务器数据库防拷贝监控
	 SCMODULE_NETOFFLINEMON=8//网络离线监控
};

//处理方式定义
#define ACTION_STUDY 0x0000  //学习
#define ACTION_DENY  0x0001  //禁用
#define ACTION_PASS  0x0002  //启用

typedef struct tagClientGlobalParaEx
{
	char center_id[GLOBALPARA_CENTERID_LEN+1]; //控制中心ID
	char center_name[GLOBALPARA_CENTERNAME_LEN+1]; //控制中心名称
	char manage_ip[BDICSP_IPADDR_LEN+1];//管理通道IP
	UInt16 manage_port;// 管理通道PORT
	char log_ip[BDICSP_IPADDR_LEN+1];;//管理通道IP
	UInt16 log_port;// 管理通道PORT
	char loginpw[BDICSP_PW_LEN+1];//登陆密码
	UInt64 sc_modules;//监控模块开关
	UInt64 sc_options;//监控选项开关(预留)
} bdicsp_client_globalpara_t;


//ARP监控参数
typedef struct tagArpmonParaEx
{
	UInt32 NacDenySw; //非法阻断开关
	UInt32 NacDenyCycle;//非法阻断周期(分钟)
	UInt32 NacDenyTime;//'阻断持续时间(分钟)
	UInt32 ScanSw;//定时扫描开关
	UInt32 ScanCycle;//定时扫描周期(分钟)
	UInt32 AlarmCycle;//违规报警周期(秒)
	UInt32 IpmacBindSw;// IP/MAC绑定开关
} bdicsp_arpmon_para_t;

//ARP监控子网策略
typedef struct tagArpMonSubnetPolicyEx
{
   UInt32 subnet_id;//子网ID
   char subnet_addr[BDICSP_IPADDR_LEN+1];//子网IP
   char subnet_mask[BDICSP_IPADDR_LEN+1];//子网掩码
   char gateway_addr[BDICSP_IPADDR_LEN+1];//网关IP
   UInt64 policy_groupid;// 策略组ID
   UInt32 study_flag;//学习标识
} bdicsp_arpmon_subnet_t;

//合法设备策略
typedef struct  tagValidDevPoliceEx
{
    UInt32 policy_id; //策略ID
	UInt64  group_id; //策略组ID
	char dev_mac[BDICSP_MAC_LEN+1]; //设备MAC
	char dev_ipaddr[BDICSP_IPADDR_LEN+1]; //设备IP
	UInt8  dev_valid_sw; //设备有效时间开关
	UInt64  dev_valid_starttime;//有效时间起始时间
	UInt64  dev_valid_stoptime;//有效时间终止时间
	UInt32  dev_type;//设备类型　
	char dev_no[BDICSP_DEVNO_LEN+1];//设备编号
	char dev_name[BDICSP_DEVNAME_LEN+1];//设备名称 
	char deptname[BDICSP_DEPTNAME_LEN+1];//所属部门   
	char useman[BDICSP_USEMAN_LEN+1];//使用人  
	char dev_pos[BDICSP_DEVPOS_LEN+1];//设备位置 
} bdicsp_nacmon_validdev_policy_t;

//usb监控参数
#define USBMON_METHOD_PROMPT 0x01 //客户端提示
#define USBMON_METHOD_POPUP  0x02 //弹出
#define USBMON_METHOD_FORMAT 0x04 //格式化
#define USBMON_METHOD_UPDATE 0x08 //重新扫描
typedef struct tagUsbmonParaEx
{
	UInt16 deal_method;//违规处理方式	
} bdicsp_usbmon_para_t;


//usb监控策略
typedef struct tagUsbmonPolicyEx
{
	UInt32 policy_id; //策略ID
	UInt64 group_id; //策略组ID
    UInt32 usb_classify_id;//USB分类值
    char deptname[BDICSP_DEPTNAME_LEN+1];// 使用部门
    char useman[BDICSP_USEMAN_LEN+1];//使用人
    char dev_number[BDICSP_DEVNO_LEN+1];// 设备编号
    char dev_vid[BDICSP_DEVVID_LEN+1];// 设备VID
    char dev_pid[BDICSP_DEVPID_LEN+1];// 设备PID
    char dev_uid[BDICSP_DEVUID_LEN+1];// 设备UID
	UInt8 action;// 授权策略
	UInt8 logsw;//是否记录日志
	UInt8 alert_level;//告警级别
	UInt32 alert_method;//告警方式
} bdicsp_usbmon_policy_t;
 

//非法连接其它设备监控参数
#define PSNETBHMON_METHOD_PROMPT 0x01 //客户端提示
#define PSNETBHMON_METHOD_OFFNET  0x02 //断网
#define PSNETBHMON_METHOD_SHUTDOWN 0x04 //关机
typedef struct tagPsNetbhmonParaEx
{
	UInt16 deal_method;//违规处理方式	
} bdicsp_psnetbhmon_para_t;

//非法连接其它设备监控策略
typedef struct tagPsNetbhmonPolicyEx
{
	UInt32 policy_id; //策略ID
	UInt64 group_id; //策略组ID
	UInt8 action;// 授权策略
	char ps_name[BDICSP_PSNAME_LEN+1];// 进程名称
    UInt8 match_fullpath_sw;//是否匹配全路径
    char ps_path[BDICSP_PSPATH_LEN+1];// 进程路径
    char rmdev_ipaddr[BDICSP_IPADDR_LEN+1];// 远程设备地址
    UInt16 rmdev_port;// 远程设备端口
	UInt8 logsw ;//是否记录日志
	UInt8 alert_level;//告警级别
	UInt32 alert_method;//告警方式
} bdicsp_psnetbhmon_policy_t;
 
//非法外联监控参数
#define OUTCONNMON_METHOD_PROMPT 0x01 //客户端提示
#define OUTCONNMON_METHOD_OFFNET  0x02 //断网
#define OUTCONNMON_METHOD_SHUTDOWN 0x04 //关机
typedef struct tagExtractConnMonParaEx
{
	UInt16 deal_method;//违规处理方式	
} bdicsp_extractconnmon_para_t;

//非法外联监控策略
#define OUTCONNMON_SUBNETDESC_LEN 32

//子网类型定义
#define OUTCONNMON_SUBNETTYPE_PUBLIC 1 //公网
#define OUTCONNMON_SUBNETTYPE_PRIVATE 2 //私网
/*　私网地址范围
//1.A类地址: IP:10.0.0.0 掩码:255.0.0.0
//2.B类地址: IP:172.16.0.0 掩码:255.240.0.0 
//3.C类地址: IP:192.168.0.0 掩码:255.255.0.0 
*/
typedef struct tagExtractConnMonPolicyEx
{
	UInt32 policy_id; //策略ID
	UInt64 group_id; //策略组ID
	UInt8 action;// 授权策略
	char  subnet_desc[OUTCONNMON_SUBNETDESC_LEN+1];// 子网描述
    UInt8 subnet_type;// 子网类型
    char subnet_ipaddr[BDICSP_IPADDR_LEN+1];// 子网IP(0.0.0.0代表所有ＩＰ),网段（子网ＩＰ）、域名、主机ＩＰ
    char subnet_mask[BDICSP_IPADDR_LEN+1];// 子网掩码（0.0.0.0代表）
	UInt8 logsw;//是否记录日志
	UInt8 alert_level;//告警级别
	UInt32 alert_method;//告警方式
} bdicsp_extractconnmon_policy_t;

//文件防火墙监控策略

//文件操作方式定义
#define FILEMON_METHOD_READ			0x01 //读
#define FILEMON_METHOD_WRITE		0x02 //写
#define FILEMON_METHOD_CREATE		0x04 //创新
#define FILEMON_METHOD_DEL 			0x08 //删除 
#define FILEMON_METHOD_RENAME		0x10 //改名
#define FILEMON_METHOD_COPY			0x20 //拷贝(预留）
#define FILEMON_METHOD_PRINT		0x40 //打印 (预留）
#define FILEMON_METHOD_MODS			0x80 //修改属性或权限(预留）
typedef struct tagFilemonPolicyEx
{
	UInt32 policy_id; //策略ID
	UInt64 group_id; //策略组ID
	UInt8  action;// 授权策略
    char file_path[BDICSP_PSPATH_LEN+1];// 监控文件全路径
    UInt16 file_method;//文件操作方式（读写、新建、删除、改名等）
	char ps_name[BDICSP_PSNAME_LEN+1];// 进程名称
    UInt8 match_fullpath_sw;// 是否匹配全路径
    char ps_path[BDICSP_PATH_LEN+1];// 进程文件路径
	UInt8 logsw ;//是否记录日志
	UInt8 alert_level;//告警级别
	UInt32 alert_method;//告警方式
} bdicsp_filemon_policy_t;

//服务器存储单元监控策略
typedef struct tagHdmonPolicyEx
{
	UInt32 policy_id; //策略ID
	char dev_name[BDICSP_DEVNAME_LEN+1]; //磁盘名称
    char dev_vid[BDICSP_DEVVID_LEN+1]; //磁盘VID
    char dev_pid[BDICSP_DEVPID_LEN+1]; //磁盘PID
    char dev_uid[BDICSP_DEVUID_LEN+1]; //磁盘UID
	UInt8 logsw;//是否记录日志
	UInt8 alert_level;//告警级别(预留)
	UInt32 alert_method;//告警方式(预留)
} bdicsp_hdmon_policy_t;

//数据库防拷贝联动参数
typedef struct tagNacLinkageParaEx
{
	UInt8 linkage_sw ;//联动启用开关
	UInt8 linkage_method;//联动方式
} bdicsp_naclinkage_para_t;

//数据库防拷贝准入控制联动策略
//禁止主机远程访问策略
typedef struct tagNacLinkagePolicyEx
{
	UInt32 policy_id; // 策略ID
	char src_ipaddr[BDICSP_IPADDR_LEN+1];//源主机IP
	char rem_ipaddr[BDICSP_IPADDR_LEN+1];//目标主机IP
	char src_mac[BDICSP_MAC_LEN+1];//源主机MAC
	char rem_mac[BDICSP_MAC_LEN+1];//目标主机MAC 
	UInt8 logsw;//是否记录日志(预留)
	UInt8 alert_level;//告警级别(预留)
	UInt32 alert_method;//告警方式(预留)
} policypkt_naclinkage_t;
//主机总体监控策略


//启用监控策略
enum ScPolicyType 
{   
	NETOFFLINEMON_POLICY = 1,//服务器/网络离线监控策略
	NACMON_POLICY = 2,//非法主机接入监控策略
	USBMON_POLICY = 3,//USB端口配置监控策略
	USBSTORMON_POLICY = 4,//移动介质监控策略
	PSNETBHMON_POLICY = 5,//非法连接其他设备监控策略
	EXTRACTCONNMON_POLICY = 6,//非法连接外网监控策略
	HDMON_POLICY = 7,//服务器存储单元监控策略
	DBAUTHMON_POLICY = 8,//服务器数据库防拷贝监控策略
	NETVIRUSMON_POLICY = 9 //网络病毒监控策略	
};
//子策略类型
enum ScSubPolicyType
{
    DEFAULT_SUBPOLICY = 0x00, //该主类型默认子策略
	SWITCH_PORT_SUBPOLICY = 0x21, //交换机端口子策略
    SWITCH_DEV_SUBPOLICY =  0x22, //交换机设备子策略
	ARPMON_SUBNET_SUBPOLICY = 0x23, // ARP监控子网子策略
	DBMON_LINKAGE_SUBPOLICY = 0x24  //数据库防拷贝联动子策略
};

typedef struct tagStartScPolicyEx
{
    UInt8 main_policy;
    UInt8 sub_policy;	
} bdicsp_start_policy_t;

//停止监控策略
typedef struct tagStopScPolicyEx
{
     UInt8 main_policy;
     UInt8 sub_policy;	
} bdicsp_stop_policy_t;

//告警类型定义
enum BDScAlertType
{
	NETOFFLINEMON_ALERT = 1,//服务器/网络离线监控告警
	NACMON_ALERT = 2,//非法主机接入监控告警
	USBMON_ALERT = 3,//USB端口配置监控告警
	USBSTORMON_ALERT = 4,//移动介质监控告警
	PSNETBHMON_ALERT = 5,//非法连接其他设备监控告警
	EXTRACTCONNMON_ALERT = 6,//非法连接外网监控告警
	HDMON_ALERT = 7,//服务器存储单元监控告警
	DBAUTHMON_ALERT = 8,//服务器数据库防拷贝监控告警
	NETVIRUSMON_ALERT = 9 //网络病毒监控告警	
};

//告警参数类型定义
enum BDScAlertParaType
{
    ALERTPARA_LEVEL=1, //告警级别
    ALERTPARA_METHOD=2, //告警方式
    ALERTPARA_CONTENT=3, //告警内容
    ALERTPARA_AUTOCLEAR=4//告警自动清除
};

//告警级别参数
typedef struct tagAlertLevelParaEx
{
    UInt32 alert_cg_id; //告警类型
    UInt8 alert_level; //告警级别	
} bdicsp_alert_levelpara_t;


//告警自动清除参数
typedef struct tagAlertAutoClearParaEx
{
    UInt32 alert_cg_id;// 告警ID
    UInt8 ac_sw;// 自动清除开关
    UInt32 ac_timelimit;// 自动清除期限(秒)	
} bdicsp_alert_clearpara_t;

//告警方式参数
typedef struct tagAlertMethodParaEx
{
	UInt8 alert_level; //告警级别	
	UInt32 alert_method;//告警方式
} bdicsp_alert_methodpara_t;

//监控日志标识
#define LOG_FLAG_AUTOCLEAR 0x01 //实时告警自动清除

//监控日志类型定义
enum BDScLogType
{
	NETOFFLINEMON_LOG = 1,//服务器/网络离线监控日志
	NACMON_LOG = 2,//非法主机接入监控日志
	USBMON_LOG = 3,//USB端口配置监控日志
	USBSTORMON_LOG = 4,//移动介质监控日志
	PSNETBHMON_LOG = 5,//非法连接其他设备监控日志
	EXTRACTCONNMON_LOG = 6,//非法连接外网监控日志
	HDMON_LOG = 7,//服务器存储单元监控日志
	DBAUTHMON_LOG = 8,//服务器数据库防拷贝监控日志
	NETVIRUSMON_LOG = 9 //网络病毒监控日志	
};

//服务器/网络离线监控日志
#define SUBNET_DESC_LEN 40
typedef struct tagNetOfflinemonLogEx
{
	UInt64 logtime;//记录时间
	UInt8 alert_level;//告警级别
	UInt32 log_flag;//日志标识(LOG_FLAG_xxx)
	char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
	UInt64 policy_groupid;// 策略组ID
	UInt64 policy_id;// 策略ID
	char useman[BDICSP_USEMAN_LEN+1];// 使用人
	char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
	char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
	UInt16 detect_method;//探测方式
	UInt16 detect_cycle;//探测周期(秒)
	UInt16 detect_times;//连续周期数
	UInt16 delay_toplimit;//时延阀值(秒)
	UInt8 dt_status;//时延状态(1-正常，0-超时)
	UInt8 status;//状态(1-在线，0-离线)
	char  subnet_ipaddr[BDICSP_IPADDR_LEN+1];//服务器/子网IP
	char subnet_desc[SUBNET_DESC_LEN+1];//服务器/子网描述
} bdicsp_netofflinemon_log_t;

// USB监控日志
typedef struct tagUsbmonLogEx
{
    UInt64 logtime;//记录时间
    UInt8 alert_level;//告警级别
    UInt32 log_flag;//日志标识(LOG_FLAG_xxx)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt8 action;// 动作
    UInt8 usb_type;// USB类型
    char dev_name[BDICSP_DEVNAME_LEN+1];// 设备名称
    char dev_vid[BDICSP_DEVVID_LEN+1];// 设备VID
    char dev_pid[BDICSP_DEVPID_LEN+1];// 设备PID
    char dev_uid[BDICSP_DEVUID_LEN+1];// 设备UID
} bdicsp_usbmon_log_t;

// USB存储监控日志
#define USBSTORMON_ACTION_LOGIN  1 //登陆
#define USBSTORMON_ACTION_LOGOUT 2 //登出
#define USBSTORMON_ACTION_CHPW	 3 //修改密码
#define USBSTORMON_ACTION_DENY   4 //非法接入
#define USBSTORMON_ACTION_PASS   5 //合法接入
#define USBSTORMON_ACTION_CREATEFILE   6 //新建文件
#define USBSTORMON_ACTION_WRITEFILE     7 //新建文件
#define USBSTORMON_ACTION_REMOVEFILE    8  //删除文件
#define USBSTORMON_ACTION_RENFILE    9  //文件改名
#define USBSTORMON_ACTION_READFILE    10 //读取文件
#define USBSTORMON_ACTION_ETC    255 //其它方式
typedef struct tagUsbstormonLogEx
{
    UInt64 logtime;//记录时间
    UInt8 alert_level;//告警级别
	UInt32 log_flag;//日志标识(0x01表示实时告警自动清除)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt8 action;// 动作
    char dev_name[BDICSP_DEVNAME_LEN+1];// 设备名称
    char dev_vid[BDICSP_DEVVID_LEN+1];// 设备VID
    char dev_pid[BDICSP_DEVPID_LEN+1];// 设备PID
    char dev_uid[BDICSP_DEVUID_LEN+1];// 设备UID
    char useman[BDICSP_USEMAN_LEN+1];//使用人
    char content[BDICSP_PATH_LEN+1];// 操作内容/文件名
} bdicsp_usbstormon_log_t;

//主机非法接入监控日志

//处理方式定义
#define NACMON_DEALMETHOD_STUDY 0x00 //学习
#define NACMON_DEALMETHOD_ALERT 0x01 //告警
#define NACMON_DEALMETHOD_DENY  0x02 //拦截

//监控方式定义
#define NACMON_SCMETHOD_ARP 0x01
#define NACMON_SCMETHOD_SNMP 0x02


typedef struct tagNacmonLogEx
{
    UInt64 logtime;//记录时间
    UInt8 alert_level;//告警级别
	UInt32 log_flag;//日志标识(0x01表示实时告警自动清除)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt32 deal_method;//处理方式
    UInt32 sc_method;//监控方式
    char sensor_ip[BDICSP_IPADDR_LEN+1];// 探针IP
    char sensor_desc[BDICSP_IPADDR_LEN+1];// 探针描述
} bdicsp_nacmon_log_t;

//主机非法连接其它设备日志
typedef struct tagPsnetbhmonLogEx
{
    UInt64 logtime;//记录时间
    UInt8  alert_level;//告警级别
	UInt32 log_flag;//日志标识(0x01表示实时告警自动清除)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt8 deal_method ;//处理方式
    char ps_name[BDICSP_PSNAME_LEN+1];// 进程名称 
    UInt8 match_fullpath_sw;// 匹配全路径
    char ps_path[BDICSP_PSPATH_LEN+1];// 进程路径
    char rmdev_ipaddr[BDICSP_IPADDR_LEN+1];// 远程设备地址
    UInt16 rmdev_port;// 远程设备端口
} bdicsp_psnetbhmon_log_t;

//主机非法外联实监控日志
typedef struct tagExtracConnMonLogEx 
{
    UInt64 logtime;//记录时间
    UInt8 alert_level;//告警级别
	UInt32 log_flag;//日志标识(0x01表示实时告警自动清除)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt8 action;// 动作
    char net_intf[BDICSP_INTF_LEN+1];// 网络接口          
	char remnet_addr[BDICSP_IPADDR_LEN+1];// 外网地址
} bdicsp_extractconnmon_log_t;

//服务器存储单元监控日志
#define HDMON_EVENT_STUDY 0x00
#define HDMON_EVENT_ADD 0x01
#define HDMON_EVENT_DEL 0x02
typedef struct tagHdmonLogEx
{
    UInt64 logtime;//记录时间
    UInt8 alert_level;//告警级别
	UInt32 log_flag;//日志标识(0x01表示实时告警自动清除)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt8 event_type;//事件类型(0x00-自动学习0x01-增加 0x02-减少 0x03更换
    char dev_name[BDICSP_DEVNAME_LEN+1];// 设备名称
    char dev_vid[BDICSP_DEVVID_LEN+1];// 设备VID
    char dev_pid[BDICSP_DEVPID_LEN+1];// 设备PID
    char dev_uid[BDICSP_DEVUID_LEN+1];// 设备UID
} bdicsp_hdmon_log_t;
//数据库防拷贝监控日志
#define BDICSP_DBAUDIT_EVENTLOG_LEN 4096

//事件类型
#define DBAUDITMON_EVENTTYPE_FILEMON 1 //文件监控（本地文件拷贝）
#define DBAUDITMON_EVENTTYPE_DBAUDIT 2 //数据库审计（远程读取）
//处理方式
#define DBAUDITMON_DEALMETHOD_ALERT 1
#define DBAUDITMON_DEALMETHOD_DENY 2

typedef struct tagDbauditMonLogEx
{
	UInt64 logtime;//记录时间
    UInt8 alert_level;//告警级别
	UInt32 log_flag;//标识(0x01表示实时告警自动清除)
    char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
    UInt64 policy_groupid;// 策略组ID
    UInt64 policy_id;// 策略ID
    char username[BDICSP_USEMAN_LEN+1];// 员工姓名
    char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
    char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
    UInt8 deal_method;//处理方式 //ACTION_xxx
    UInt8 event_type;//事件类型
    char event_desc[BDICSP_DBAUDIT_EVENTLOG_LEN+1];//事件描叙
	char file_path[BDICSP_PSPATH_LEN+1];// 监控文件全路径
	UInt16 file_method;//文件操作方式（读写、新建、删除、改名等）
	char ps_name[BDICSP_PSNAME_LEN+1];// 进程名称
	UInt8 match_fullpath_sw;// 是否匹配全路径
	char ps_path[BDICSP_PATH_LEN+1];// 进程文件路径
} bdicsp_dbauditmon_log_t;

//病毒网关日志
#define BDICSP_VIRUSDESC_LEN 512
typedef struct tagNetVirusMonLogEx
{
	UInt64 logtime;//记录时间
	UInt8 alert_level;//告警级别
	UInt32 log_flag;//标识(0x01表示实时告警自动清除)
	char area_code[BDICSP_AREACODE_LEN+1];// 地区编号          
	UInt32 dept_id;// 部门ID
	UInt64 policy_groupid;// 策略组ID
	UInt64 policy_id;// 策略ID
	char username[BDICSP_USEMAN_LEN+1];// 员工姓名
	char host_ip[BDICSP_IPADDR_LEN+1];// 主机IP
	char host_mac[BDICSP_MAC_LEN+1];// 主机MAC
	UInt8 deal_method;//处理方式 //ACTION_xxx
	UInt16 virus_type;//病毒类型
	char virus_desc[BDICSP_VIRUSDESC_LEN+1];//病毒描叙
	char fw_ip[BDICSP_IPADDR_LEN+1];// 病毒网关ip
	char virsrc_ip[BDICSP_IPADDR_LEN+1];// 病毒源主机IP
} bdicsp_netvirusmon_log_t;


//传送文件报文
//文件类型定义
enum BDScFileType
{
	BDSC_FILETYPE_PATCH=1,//客户端程序升级文件
	BDSC_FILETYPE_PRT=2,//打印文件
	BDSC_FILETYPE_CLIBDOC=3,//剪贴板监控文件
	BDSC_FILETYPE_SOFTDISP=4,//软件分发文件
	BDSC_FILETYPE_SECCHECK=5//安全检查文件
};

#define BDICSP_FILENAME_LEN 256
typedef struct tagDownFileHeadEx
{
	UInt32 filetype; //文件类型
	UInt32 fileno; //文件编号（从0开始）
	UInt64 pktno; //同一文件块号（从0开始）
	char filename[BDICSP_FILENAME_LEN+1]; //文件名
	UInt64 filesize; //文件大小
	UInt32 crc32; //CRC32校验和
	UInt64 datasize;//数据大小
    char filedata[0]; //文件数据 
} bdicsp_filehead_t;

//状态报文
//状态码
enum BDStatCode
{
    SUCCESS_CODE = 0,//成功
    FAILED_CODE = 1,//失败
    PWD_ERR_CODE = 2,//认证口令错误
    REPEATE_ERR_CODE = 3,//主机已经存在
	OPTSYS_ERR_CODE = 4,//该操作系统不支持	
	PARA_ERR_CODE = 5,//参数错误
	DATA_ERR_CODE = 6//数据错误
};
//状态响应报文
typedef struct tagReplyStatEx
{
    BDStatCode statcode;//返回状态码
	UInt32 dwValueSize;//返回状态值大小
	char * pStatValue;//返回状态值
	UInt32 dwDescSize;//返回状态描述大小
	char * pStatDesc;//返回状态描述	
} bdicsp_reply_stat_t;

//结果响应报文
typedef struct tagReplyResultEx
{
    BDStatCode retcode;//返回状态码
	UInt32 dwValueSize;//返回状态值大小
	char * pRetValue;//返回状态值
	UInt32 dwDescSize;//返回状态描述大小
	char * pRetDesc;//返回状态描述	
} bdicsp_reply_result_t;

//返回未分配主机ID响应报文
typedef struct  tagReplyHostidEx
{
    BDStatCode retcode;//返回状态码
    char hostid[BDICSP_HOSTID_LEN+1];//主机ID 	
} bdicsp_reply_freehostid_t;


#pragma pack(pop)

#endif //_H_COMMON_BDICSP


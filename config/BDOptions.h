/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2017,      BLUEDON
 * All rights reserved.
 *
 * @file    BDOptions.h
 * @brief
 *
 *
 * @version 1.0
 * @author  xxx
 * @date    2017年08月03日
 *
 * 修订说明：最初版本
 **************************************************/

#ifndef _BDOPTIONS_H_
#define _BDOPTIONS_H_

#include "common/common.h"
#include "common/BDIcspModuleBase.h"


#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>

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

#include "probuf/socmsg.pb.h"

#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/CipherKey.h"

using Util::IniFileConfiguration;
using Util::AbstractConfiguration;
using namespace Poco::Crypto;
using namespace bd_soc_protobuf;
/*
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/X509Certificate.h"
#include "Poco/Crypto/CryptoStream.h"
*/


typedef void (*pfnOnHostInfoChanged)();

typedef struct BDProxyConfigEx {
    char chLog4File[100];         //配置文件目录
    char chCollectServerProtocol[10]; //传感器监听协议
    char chCollectServerIP[20];   //传感器监听ip
    char chCollectServerName[50]; //传感器名字
    int  nCollectServerPort;      //传感器监听端口
    int  nHeartbeatSleepnanosec;  //心跳间隔休眠频率(微秒)
    int  nHeartbeatMonitorsec;    //监控信息上报频率(秒)
    char chRecordSep[2];          //记录分隔符
	char chFieldSep[2];           //字段分隔符
	char chModuleXml[100];
	char chReqServAddr[30];        // MS 服务器地址
	char chRespServAddrA[30];      // AS 服务器地址	(日志类)
	char chRespServAddrB[30];      // AS 服务器地址	(监控类)
	char chRespServAddrC[30];      // AS 服务器地址	(业务类)
    char chRespServAddrD[30];      // AS 服务器地址	(流量类)
    char chRespServAddrE[30];      // AS 服务器地址	(netflow流量类)
	bool bAutoStartModule;
    int  nCompressControl;          //back to front; 0:no compressed, 1:compressed
    int  nEncryptControl;           //back to front; 0:no encrypted, 1:encrypted

}tag_proxy_config_t,*tag_proxy_config_pt;


#define BD_SOC_BASE_CFG         "bd_soc_agent.cfg"
#define CONFIGURE_RELATIVE_DIR  "conf/"
#define LIB_RELATIVE_DIR        "lib/"

//读取xml 数据宏定义
#define GET_CONF_ITEM_STRING(from,to)\ 
		if( pConf->hasProperty(#from) ) {\
			string strValue = pConf->getString(#from);\
			to = strValue;\		
 		}  

#define GET_CONF_ITEM_INT(from,to)\ 
		if( pConf->hasProperty(#from) ) {\
			int nValue = pConf->getInt(#from);\
			to = nValue;\		
 		}  

#define GET_CONF_ITEM_CHAR(from,to,size)\ 
		if( pConf->hasProperty(#from) ) {\
			string strValue = pConf->getString(#from);\		
			BDSTRING::strlcpy(to,strValue.c_str(),size+1);\	
 		}  

#define GET_CONF_ITEM_BOOL(from,to)\ 
		if( pConf->hasProperty(#from) ) {\
			bool bValue = pConf->getBool(#from);\
			to = bValue;\		
 		}  



class CBDOptions:public CBdIcspModuleBase
{
public:
	CBDOptions();
	virtual ~CBDOptions();

    virtual void Init(void); //初始化配置路径
    virtual bool Start(void); //开始加载配置
    virtual void Stop(void);
	static CBDOptions * instance();
    static void exit();      //释放资源

public:
    void inline SetHostInfoCallback(pfnOnHostInfoChanged pFunc)  //设置回调函数
    {m_pfnHostInfoCallback = pFunc;}
	
 	void SetAppConfig(const std::string strAppConfig); //设置应用程序配置文件
	string GetAppPath(void);//得到应用程序路径
 
public:
	inline const tag_proxy_config_pt GetConfig()  {return &m_agentConfig;}
	
	
    bool setCipher();//设置加密方式,加密前必须调用
    bool Cipherdestroy();//销毁加密cipther
    string DecodePw(const string strSrcPw, Message_enptypeNo Ciphernum );//解密口令
    string EncodePw(const string strSrcPw, Message_enptypeNo Ciphernum );//加密口令

protected:
    bool Load(void);  //初始化加密对象，初始化log4cxx
	void Clear(void);
    bool LoadBaseCfg(); //读取配置文件中配置
	

private:
	static CBDOptions  * m_instance;
	static Poco::FastMutex  m_InsMutex;
	//Poco::Crypto::Cipher::Ptr m_pCipher;
	

    map<int, Cipher::Ptr> Cipher_map; //加密cipher保存
    map<int, CipherKey *> CipherKey_map; //加密CipherKey保存

	

	
	
	pfnOnHostInfoChanged m_pfnHostInfoCallback;

	string m_strAppPath;          //应用路径
	string m_strAppConfigFile;    //带有完整路径的配置文件名称

    tag_proxy_config_t m_agentConfig;  //配置文件对象


};

#endif //_BDOPTIONS_H_



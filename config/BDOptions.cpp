#include  "unistd.h"
#include "BDOptions.h"
#include "utils/bdstring.h"
#include "probuf/socmsg.pb.h"
//#include "utils/snprintf.h"
#include "crypt/aes.h"
//#include "crypt/Base64.h"
//Crypto
#include "Poco/Crypto/CipherFactory.h"
#include "Poco/Crypto/Cipher.h"
#include "Poco/Crypto/CipherKeyImpl.h"
#include "Poco/Crypto/CipherKey.h"
#include "Poco/Crypto/CryptoStream.h"


#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace Poco::Crypto;
//using namespace log4cxx;
using namespace Poco;
using namespace std;
using namespace bd_soc_protobuf;

FastMutex  CBDOptions::m_InsMutex;
CBDOptions  * CBDOptions::m_instance=NULL;


#if defined(POCO_OS_FAMILY_WINDOWS)
#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#endif

//配置秘钥
static unsigned char AesKey[32]=
{
    0x42,0xd2,0x7a,0x73,0xfc,0x37,0xe2,0x8d,\
    0xd2,0x70,0x5f,0x54,0x41,0x28,0x91,0x77,\
    0x59,0x8f,0x55,0x99,0x93,0x53,0xd7,0x17,\
    0x53,0x35,0x1f,0x03,0x71,0x0d,0xc5,0xd5
};

/*
unsigned char i[] = { 219, 157, 180, 130, 179, 204, 171, 242, 251, 112, 209,
            173, 100, 219, 141, 179 };

unsigned char k[] = { 49, 17, 199, 69, 242, 178, 153, 25, 129, 86, 8, 125,
                151, 171, 20, 162, 4, 247, 64, 221, 42, 57, 80, 69, 48, 164, 8, 34,
                44, 24, 9, 90 };
*/

CBDOptions::CBDOptions()
{
    CipherKey_map.clear();
    Cipher_map.clear();

    m_strAppPath="";
    memset(&m_agentConfig,0,sizeof(m_agentConfig));

    m_pfnHostInfoCallback=NULL;
}

//
CBDOptions::~CBDOptions()
{
        //释放加解密对象空间
    for(map<int, CipherKey *>::iterator iter=CipherKey_map.begin();iter!=CipherKey_map.end();++iter)
    if(iter->second != NULL)
    {
        delete iter->second;
        iter->second = NULL;
    }

}

bool CBDOptions::Cipherdestroy()//销毁加密cipther
{
    for(map<int, CipherKey *>::iterator iter=CipherKey_map.begin();iter!=CipherKey_map.end();++iter)
    if(iter->second != NULL)
    {
        delete iter->second;
        iter->second = NULL;
    }
    return true;
}

void CBDOptions::Init(void)
{
    //将完整配置路径转换为应用程序路径
    m_strAppPath = m_strAppConfigFile.substr(0,m_strAppConfigFile.rfind(CONFIGURE_RELATIVE_DIR));
    cout<<m_strAppPath<<endl;
    cout<<m_strAppConfigFile<<endl;
}

//单例
CBDOptions * CBDOptions::instance()
{
    if(m_instance==NULL)
    {
        FastMutex::ScopedLock lock(m_InsMutex);
        if(m_instance==NULL)
            m_instance = new CBDOptions();
    }

    return m_instance;
}

//释放资源
void CBDOptions::exit()
{
    FastMutex::ScopedLock lock(m_InsMutex);
    if(m_instance!=NULL)
    {
        delete m_instance;
        m_instance=NULL;
    }

}

//设置路径
void CBDOptions::SetAppConfig(const std::string strAppConfig)
{
    m_strAppConfigFile = strAppConfig;
}
//设置应用程序配置文件

//获取路径
string CBDOptions::GetAppPath(void)
{
    return m_strAppPath;
}

//初始化加密对象，初始化log4cxx
bool CBDOptions::Load(void)
{
    //初始化aes加密
    if(!setCipher()){
        cerr<<" error: setCipher() fails !"<<endl;
		return false;
    }

    cout<<" Enter CBDOptions::Load()..."<<endl;
    if ( !LoadBaseCfg() ) {
        cerr<<" error: LoadBaseCfg() fails !"<<endl;
        return false;
    }

    // log4xx
    if( strlen(m_agentConfig.chLog4File) > 0 ) {
        if(access(m_agentConfig.chLog4File, F_OK) != 0) {
            cerr<<"error: "<<m_agentConfig.chLog4File<<" not exist !"<<endl;
            return false;
        } else {
             try {
                log4cxx::PropertyConfigurator::configure(m_agentConfig.chLog4File);
             } catch (Exception &) {
                 cerr << "error: log4cxx init failed !" << endl;
                 return false;
             }
        }
    } else {
        return false;
    }

    return true;
}

//读取配置文件中配置
bool CBDOptions::LoadBaseCfg()
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
         GET_CONF_ITEM_CHAR(log4configfile,m_agentConfig.chLog4File,100);
         GET_CONF_ITEM_CHAR(collect_server_protocol,m_agentConfig.chCollectServerProtocol,10);
         GET_CONF_ITEM_CHAR(collect_server_ip,m_agentConfig.chCollectServerIP,20);
         GET_CONF_ITEM_CHAR(collect_server_name,m_agentConfig.chCollectServerName,50);
         GET_CONF_ITEM_INT(collect_server_port,m_agentConfig.nCollectServerPort);
         GET_CONF_ITEM_INT(heartbeat_sleepnanosec,m_agentConfig.nHeartbeatSleepnanosec);
         GET_CONF_ITEM_INT(heartbeat_monitorsec,m_agentConfig.nHeartbeatMonitorsec);
         GET_CONF_ITEM_CHAR(record_separator,m_agentConfig.chRecordSep,2);
         GET_CONF_ITEM_CHAR(field_separator,m_agentConfig.chFieldSep,2);
         GET_CONF_ITEM_CHAR(module_xml,m_agentConfig.chModuleXml,100);
         GET_CONF_ITEM_CHAR(managerserver_address,m_agentConfig.chReqServAddr,30);
         GET_CONF_ITEM_CHAR(analysisserver_address_a,m_agentConfig.chRespServAddrA,30);
         GET_CONF_ITEM_CHAR(analysisserver_address_b,m_agentConfig.chRespServAddrB,30);
         GET_CONF_ITEM_CHAR(analysisserver_address_c,m_agentConfig.chRespServAddrC,30);
         GET_CONF_ITEM_CHAR(analysisserver_address_d,m_agentConfig.chRespServAddrD,30);
         GET_CONF_ITEM_CHAR(analysisserver_address_e,m_agentConfig.chRespServAddrE,30);
         GET_CONF_ITEM_BOOL(auto_start_module,m_agentConfig.bAutoStartModule);
         GET_CONF_ITEM_INT(compress_control,m_agentConfig.nCompressControl);
         GET_CONF_ITEM_INT(encrypt_control,m_agentConfig.nEncryptControl);
    }
    catch (NotFoundException& exc ) {
        cerr << "LoadBaseCfg:" << exc.displayText() << endl;
        return false;
    } catch (SyntaxException& exc) {
        cerr << "LoadBaseCfg:" << exc.displayText() << endl;
        return false;
    }
    return true;
}

void CBDOptions::Clear(void)
{
    //FastMutex::ScopedLock lock(m_DeptInfoMutex);
    return;
}
//入口
bool CBDOptions::Start(void)
{
    bool bRet=Load();
    if(!bRet) return false;

    return true;
}

void CBDOptions::Stop(void)
{
    Clear();
}

//解密口令
string CBDOptions::DecodePw(const string strSrcPw, Message_enptypeNo Ciphernum)
{

    int nSrcLen=strSrcPw.length();
    if(nSrcLen<=0)
        return "";

    if(CipherKey_map[Ciphernum] == NULL)
        return strSrcPw;

    string strDstPw = Cipher_map[Ciphernum]->decryptString(strSrcPw, Cipher::ENC_BASE64);


    return strDstPw;
}


//加密口令
string CBDOptions::EncodePw(const string strSrcPw, Message_enptypeNo Ciphernum)
{
    int nSrcLen=strSrcPw.length();
    if(nSrcLen<=0)
        return "";

    if(CipherKey_map[Ciphernum] == NULL)
        return strSrcPw;
    //string Cipher = "aes-256-ecb";
    //static CipherKey key(Cipher);
    //key.setKey( AesKey2 );
    //static Cipher::Ptr pCipher = CipherFactory::defaultFactory().createCipher(*ptr_key);

    string strDstPw = Cipher_map[Ciphernum]->encryptString(strSrcPw, Cipher::ENC_BASE64);


    return strDstPw;
}

//初始化加密对象
bool CBDOptions::setCipher()
{
    Cipher_map.clear();
    CipherKey_map.clear();
/*
1:AES256ecb
2:AES256cbc
3:AES256cfb
4:AES256ctr
5:AES256ofb
*/

    Cipher::ByteVec key = Cipher::ByteVec(AesKey, AesKey + 32);
    Cipher::ByteVec iv = Cipher::ByteVec(AesKey, AesKey + 16);
    //1: aes-256-ecb
    try {
        CipherKey_map[Message_enptypeNo_AES256ECB] = new CipherKey("aes-256-ecb");
        CipherKey_map[Message_enptypeNo_AES256ECB]->setKey( key );
        Cipher_map[Message_enptypeNo_AES256ECB] = CipherFactory::defaultFactory().createCipher(*CipherKey_map[Message_enptypeNo_AES256ECB]);

    }
    catch (Exception& exc) {
        this->Cipherdestroy();
        cerr << "aes-256-ecb setCipher:" << exc.displayText() << endl;
        return false;
    }

    //2: aes-256-cbc
    try {
        CipherKey_map[Message_enptypeNo_AES256CBC] = new CipherKey("aes-256-cbc");
        CipherKey_map[Message_enptypeNo_AES256CBC]->setKey( key );
        CipherKey_map[Message_enptypeNo_AES256CBC]->setIV( iv );
        Cipher_map[Message_enptypeNo_AES256CBC] = CipherFactory::defaultFactory().createCipher(*CipherKey_map[Message_enptypeNo_AES256CBC]);

    }
    catch (Exception& exc) {
        this->Cipherdestroy();
        cerr << "aes-256-cbc setCipher :" << exc.displayText() << endl;
        return false;
    }

    //3: aes-256-cfb
    try {
        CipherKey_map[Message_enptypeNo_AES256CFB] = new CipherKey("aes-256-cfb");
        CipherKey_map[Message_enptypeNo_AES256CFB]->setKey( key );
        CipherKey_map[Message_enptypeNo_AES256CFB]->setIV( iv );
        Cipher_map[Message_enptypeNo_AES256CFB] = CipherFactory::defaultFactory().createCipher(*CipherKey_map[Message_enptypeNo_AES256CFB]);

    }
    catch (Exception& exc) {
        this->Cipherdestroy();
        cerr << "aes-256-cfb setCipher :" << exc.displayText() << endl;
        return false;
    }

    //4: aes-256-ctr
    try {
        CipherKey_map[Message_enptypeNo_AES256CTR] = new CipherKey("aes-256-ctr");
        CipherKey_map[Message_enptypeNo_AES256CTR]->setKey( key );
        CipherKey_map[Message_enptypeNo_AES256CTR]->setIV( iv );
        Cipher_map[Message_enptypeNo_AES256CTR] = CipherFactory::defaultFactory().createCipher(*CipherKey_map[Message_enptypeNo_AES256CTR]);

    }
    catch (Exception& exc) {
        this->Cipherdestroy();
        cerr << "aes-256-ctr setCipher :" << exc.displayText() << endl;
        return false;
    }

    //5: aes-256-ofb
    try {
        CipherKey_map[Message_enptypeNo_AES256OFB] = new CipherKey("aes-256-ofb");
        CipherKey_map[Message_enptypeNo_AES256OFB]->setKey( key );
        CipherKey_map[Message_enptypeNo_AES256OFB]->setIV( iv );
        Cipher_map[Message_enptypeNo_AES256OFB] = CipherFactory::defaultFactory().createCipher(*CipherKey_map[Message_enptypeNo_AES256OFB]);

    }
    catch (Exception& exc) {
        this->Cipherdestroy();
        cerr << "aes-256-ofb setCipher :" << exc.displayText() << endl;
        return false;
    }

    return true;

}





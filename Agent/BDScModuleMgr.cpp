#include  "unistd.h"
#include "BDScModuleMgr.h"
#include "utils/bdstring.h"
#include "config/BDOptions.h"

#include "Poco/Types.h"
#include "Poco/SharedLibrary.h"
#include "Poco/Environment.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/String.h"
#include "Poco/AutoPtr.h"
#include "Poco/SAX/InputSource.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/XML/XMLException.h"

#include <iostream>

using namespace Poco;
using namespace std;

FastMutex  CBDScModuleMgr::m_InsMutex;
CBDScModuleMgr * CBDScModuleMgr::m_instance=NULL;
//模块回调函数
extern         bool BD_CALLBACK_TYPE ReportData(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData,UInt32 dwLength);
extern const void * BD_CALLBACK_TYPE  FetchData(UInt32 dwModuleId,const PModIntfDataType pDataType,UInt32& dwRetLen);

//获取模块指针
#define GET_SYMBOLR_ESULT(name,func,var,parm) \
	try {\
		if(!pShLib->hasSymbol(#name)) {\
		    cout<<" error: no symbol ["<<#name<<"]..."<<endl;\
			delete pShLib;\
			return false;\
		}\
		func fptr = (func)(pShLib->getSymbol(#name));\
		if(fptr==NULL) {\
			delete pShLib;\
			return false;\
		}\
		var = fptr(parm);\
     } catch(NotFoundException& exc) {\	    
		cout<<" error: get "<<#name<<" failed! :"<<exc.displayText()<<endl;\
		delete pShLib;\
		return false;\
	} catch(Exception & exc) {\
		cout<<" error: get "<<#name<<" failed! :"<<exc.displayText()<<endl;\
		delete pShLib;\
		return false;\
	} catch(...) {\
		cout<<" error: get "<<#name<<" failed!"<<endl;\
		delete pShLib;\
		return false;\
	}

//获取模块指针
#define GET_SYMBOL_VALUE(name,func,var) \
try {\
	if(!pShLib->hasSymbol(#name)) {\
		cout<<" error: no symbol ["<<#name<<"]..."<<endl;\
		delete pShLib;\
		return false;\
	}\
	var = (func)(pShLib->getSymbol(#name));\
	if(var==NULL) {\
		delete pShLib;\
		return false;\
	}\
 } catch(NotFoundException& exc) {\ 	
	cout<<" error: get "<<#name<<" failed! :"<<exc.displayText()<<endl;\
	delete pShLib;\
	return false;\
} catch(Exception & exc) {\
	cout<<" error: get "<<#name<<" failed! :"<<exc.displayText()<<endl;\
	delete pShLib;\
	return false;\
} catch(...) {\
	cout<<" error: get "<<#name<<" failed!"<<endl;\
	delete pShLib;\
	return false;\
}

CBDScModuleMgr::CBDScModuleMgr(void)
{
	pthread_mutex_init(&m_mtxScModuleList, NULL);
    m_ScModuleList.clear();
	strFieldSep   = CBDOptions::instance()->GetConfig()->chFieldSep;
	strRecordSep  = CBDOptions::instance()->GetConfig()->chRecordSep;
}

CBDScModuleMgr::~CBDScModuleMgr()
{
	pthread_mutex_destroy( &m_mtxScModuleList );
}
//单例
CBDScModuleMgr * CBDScModuleMgr::instance(void)
{ 
	if(!m_instance) {
		FastMutex::ScopedLock lock(m_InsMutex);
        if(!m_instance)
            m_instance = new CBDScModuleMgr();
    }

    return m_instance;
}
//销毁对象
void CBDScModuleMgr::exit()
{
	FastMutex::ScopedLock lock(m_InsMutex);
	if(m_instance!=NULL) {
		delete m_instance;
		m_instance=NULL;
	}
}
//添加模块到list
bool CBDScModuleMgr::AddScModule(bd_scmodule_t * pScModule)
{
    if(pScModule==NULL) return false;
	pthread_mutex_lock( &m_mtxScModuleList );
    m_ScModuleList.push_back(pScModule);
    AGENTLOG_INFO_S(<===============plugin information===============>);
    char chModuleId[10] = {0};
    sprintf(chModuleId,"%d",pScModule->dwModuleID);
    AGENTLOG_INFO_V(" ModuleID=>      ["+string(chModuleId)+string("]"));
    char chVersion[10] = {0};
    sprintf(chVersion,"%d",pScModule->wModuleVer);
    AGENTLOG_INFO_V(" ModuleVersion=> ["+string(chVersion)+string("]"));
    AGENTLOG_INFO_V(" ModuleName=>    ["+string(pScModule->strModuleName)+string("]"));
    if(pScModule->bIsRunning) {
        AGENTLOG_INFO_V(" RunningState=>  [true]");
    }
    else {
        AGENTLOG_INFO_V(" RunningState=>  [false]");
    }
    AGENTLOG_INFO_S(<===============plugin information===============>);
	pthread_mutex_unlock( &m_mtxScModuleList );

	return true;
}
//启动模块
bool CBDScModuleMgr::StartScModule(bd_scmodule_t * pScModule)
{
	if(pScModule==NULL)                     return false;
	if(pScModule->pScModuleClass==NULL)     return false;
    //启动之前再次确认状态
    pScModule->bIsRunning = pScModule->pScModuleClass->IsRunning();
	if(pScModule->bIsRunning)               return true;
	AGENTLOG_INFO_V("prepare starting module =>"+pScModule->strModuleName);
	if(!pScModule->pScModuleClass->Start()) return false;
	AGENTLOG_INFO_V(" finish starting module =>"+pScModule->strModuleName);		
	pScModule->bIsRunning=pScModule->pScModuleClass->IsRunning();
	return true;
}
//关闭模块
bool CBDScModuleMgr::StopScModule(bd_scmodule_t * pScModule)
{
	if(pScModule==NULL)                  return false;
	if(pScModule->pScModuleClass==NULL)  return false;
    //停止之前再次确认状态
    pScModule->bIsRunning = pScModule->pScModuleClass->IsRunning();
	if(pScModule->bIsRunning) {
		AGENTLOG_INFO_V("prepare stoping module =>"+pScModule->strModuleName);
		pScModule->pScModuleClass->Stop();
		AGENTLOG_INFO_V(" finish stoping module =>"+pScModule->strModuleName);		
	}
	else  return true;
	//可能由于停止过程较长，存在没有立即返回正确状态的情况
	pScModule->bIsRunning = pScModule->pScModuleClass->IsRunning();
	while( pScModule->bIsRunning )
	{
		pScModule->bIsRunning = pScModule->pScModuleClass->IsRunning();
		usleep(1000);
	}
    //此时默认停止成功，后续心跳中再重新获取插件状态
	return true;
}
//加载指定的模块到内存
bool CBDScModuleMgr::LoadModule(string &strModule, string &strConfig)
{
	typedef UInt32 (BD_CALLBACK_TYPE * pFunc_GetModuleId)(void);
	typedef UInt16 (BD_CALLBACK_TYPE * pFunc_GetModuleVer)(void);
	typedef char * (BD_CALLBACK_TYPE * pFunc_GetModuleName)(void);
	typedef void   (BD_CALLBACK_TYPE * pFunc_FreeScModule)(CScModuleBase * pScModuleBase);
	typedef CScModuleBase * (BD_CALLBACK_TYPE * pFunc_CreateScModule)(const string &strConfigName);

	CScModuleBase * pScModuleClass = NULL;

	string strDllPath   = CBDOptions::instance()->GetAppPath() + LIB_RELATIVE_DIR + strModule;
	string strDllConfig = CBDOptions::instance()->GetAppPath() + CONFIGURE_RELATIVE_DIR + strConfig;

    SharedLibrary * pShLib=new SharedLibrary;
    if(pShLib==NULL) return false;
    try {
        pShLib->load(strDllPath);
    } catch (NotFoundException&) {
        cout<<" error: load sharedll:"<<strDllPath.c_str()<<" failed!"<<endl;
        AGENTLOG_INFO_V(" error: load sharedll:"<<strDllPath.c_str()<<" failed!");
        delete pShLib;
        return false;
    } catch(LibraryLoadException& exc) {
        cout<<" error: load sharedll:"<<strDllPath.c_str()<<" failed! :"<<exc.displayText().c_str()<<endl;
        AGENTLOG_INFO_V(" error: load sharedll:"<<strDllPath.c_str()<<" failed!"<<exc.displayText().c_str());
        delete pShLib;
        pShLib=NULL;
        return false;
    } catch(...) {
        cout<<" error: load sharedll:"<<strDllPath.c_str()<<" failed!"<<endl;
        AGENTLOG_INFO_V(" error: load sharedll:"<<strDllPath.c_str()<<" failed!");
        delete pShLib;
        pShLib=NULL;
        return false;
    }

/*	if(!pShLib->isLoaded()) {
		delete pShLib;
		return false;
	}	*/

    //GET_SYMBOLR_ESULT(GetModuleId,pFunc_GetModuleId,dwModuleID)  
    //GET_SYMBOLR_ESULT(GetModuleVer,pFunc_GetModuleVer,wModuleVer)  
    //GET_SYMBOLR_ESULT(GetModuleName,pFunc_GetModuleName,strModuleName)  
    GET_SYMBOLR_ESULT(CreateScModule,pFunc_CreateScModule,pScModuleClass, strDllConfig) 
    pFunc_FreeScModule pFreeScModule=NULL;
	GET_SYMBOL_VALUE(FreeScModule,pFunc_FreeScModule,pFreeScModule)

    
	pScModuleClass->SetReportData(ReportData);   //callback set
	pScModuleClass->SetFetchData(FetchData);     //callback set

	if ( !pScModuleClass->Init() ) {
        AGENTLOG_ERROR_V("pScModuleClass->Init()::Error: " + strDllConfig);
		pFreeScModule(pScModuleClass);
		delete pShLib;
		return false;
	}
 
    //通过配置项决定是否自动启动模块
	if( !CBDOptions::instance()->GetConfig()->bAutoStartModule ); 
	else {
	    if(!pScModuleClass->Start()) {
		    pFreeScModule(pScModuleClass);
		    delete pShLib;
		    return false;
	    }
	}

    bd_scmodule_t * pScModule;
    try {
	    pScModule=new bd_scmodule_t;
    } catch (exception & exc){
		//pScModuleClass->Stop();
		cout<<" error : "<<exc.what()<<endl;
		pFreeScModule(pScModuleClass);
		delete pShLib;
		return false;
	}

	pScModule->dwModuleID     = pScModuleClass->GetModuleId();
	pScModule->wModuleVer     = pScModuleClass->GetModuleVersion();
	pScModule->strModuleName  = pScModuleClass->GetModuleName(); 
	pScModule->pScModuleClass = pScModuleClass;
	pScModule->bIsRunning     = pScModuleClass->IsRunning();
	pScModule->pShlib         = pShLib;
	bool bRet=AddScModule(pScModule);
	return bRet;
}
//释放内存中指定的模块
void CBDScModuleMgr::UnLoadScModule(bd_scmodule_t * pScModule)
{
	 typedef void (BD_CALLBACK_TYPE * pFunc_FreeScModule)(CScModuleBase * pScModuleBase);
	if(pScModule==NULL)
		return;
	StopScModule(pScModule);
	if(pScModule->pShlib==NULL)
		return;
	pFunc_FreeScModule pFreeScModule=NULL;
	try
	{
		pFreeScModule = (pFunc_FreeScModule)(pScModule->pShlib->getSymbol("FreeScModule"));
		if(pFreeScModule!=NULL)
		{
			pFreeScModule(pScModule->pScModuleClass);
			cout<<"FreeScModule module "<<pScModule->strModuleName<<" ok"<<endl;
		}
	}
	catch (NotFoundException&)
	{	
	//do nothing
	}
	catch (...)
	{
	//do nothing
	}
	delete pScModule->pShlib;
	delete pScModule; 
}

//读取配置文件中加载模块
bool CBDScModuleMgr::Load(void)
{
	XML::DOMParser parser;
	parser.setFeature(XML::DOMParser::FEATURE_FILTER_WHITESPACE, true);
	parser.setFeature(XML::XMLReader::FEATURE_NAMESPACES, false);
    
	string strConfFile = CBDOptions::instance()->GetConfig()->chModuleXml;
	AGENTLOG_INFO_V("loading module xml: " + strConfFile);

	XML::InputSource source(strConfFile);
	XML::Document * pDoc=NULL;
	XML::Element * pElement=NULL;
	XML::Node * pNode=NULL;
	try
	{
        pDoc = parser.parse(&source);
		if(pDoc==NULL) return false;
		pElement=pDoc->documentElement();
		if(pElement==NULL) {
			pDoc->release();
			return false;
		}
		pNode = pElement->getNodeByPath("/scmodules"); //get a element from second level 
		if(pNode==NULL) {
			pDoc->release();
			return false;
		}
	} catch (XML::XMLException& exc) {
		cerr << "CBDScModuleMgr::Load() failed" << exc.displayText() << endl;	
		if(pDoc!=NULL) pDoc->release();
		return false;
	}

	try { //ChildNode
		if(pNode->hasChildNodes()) {
			XML::NodeList* pChildNodeList=pNode->childNodes();
			unsigned long dwCount=pChildNodeList->length();
			for(UInt32 num=0; num<dwCount; num++) {
				XML::Element * pChildNode=dynamic_cast<XML::Element*>(pChildNodeList->item(num)); 
				/*
				    NodeValue 暂时不用，以后有需要再修改，
				    因为poco::xml中的Nodevalue其实是Node的子节
				    点，其其对应NodeName是#Text !
				*/
				string strDllName;
				string strConfigName;
				if( !strcmp( pChildNode->nodeName().c_str(),"module")) {			
					if( pChildNode->hasAttribute("name"))
					     strDllName = pChildNode->getAttribute("name");
					if ( pChildNode->hasAttribute("config"))		
					     strConfigName = pChildNode->getAttribute("config");
					AGENTLOG_INFO_V(" ModuleName:" + strDllName + "  ConfigName:" + strConfigName);
                    if ( !strDllName.empty() && !strConfigName.empty() ) {
					    bool bRet=LoadModule(strDllName, strConfigName);
						string strMsg = " Loading " + strDllName;
						if(bRet) strMsg += " succed!";
						else	 strMsg += " failed!";
						AGENTLOG_INFO_V(strMsg);	
						cout<<endl;
					}	
				} else {				    
				    cout<<endl;	
				}
			}
			pChildNodeList->release();
		} 
	} catch (XML::XMLException& exc) {
	    cerr << " error:CBDScModuleMgr::Load() failed:" << exc.displayText() << endl;	
		if(pDoc!=NULL) pDoc->release();
		return false;
	}	
	pDoc->release();		

	return true;
}
//清空模块list
void CBDScModuleMgr::Unload(void)
{
	if(m_ScModuleList.empty()) return;
	pthread_mutex_lock( &m_mtxScModuleList );
	AGENTLOG_INFO_V("CBDScModuleMgr::Unload start --------------------------------------------------------------------");
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL)
			continue;
		AGENTLOG_INFO_V("StopModule module A01 =>"+pScModule->strModuleName);
		UnLoadScModule(pScModule);
		AGENTLOG_INFO_V("StopModule module A02");
	}
	AGENTLOG_INFO_V("StopModule B00");
	m_ScModuleList.clear();
	AGENTLOG_INFO_V("StopModule B01");
	AGENTLOG_INFO_V("CBDScModuleMgr::Unload end ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	pthread_mutex_unlock( &m_mtxScModuleList );
}

void CBDScModuleMgr::Init(void)
{

}

//启动模块
bool CBDScModuleMgr::StartModule(UInt32 dwModuleId)
{
	bool bRet=false;
	if(m_ScModuleList.empty()) return false;
	pthread_mutex_lock( &m_mtxScModuleList );
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwModuleId) {
			bRet=StartScModule(pScModule);
			break;
		}
	}
	pthread_mutex_unlock( &m_mtxScModuleList );
	return bRet;
}

//停止模块
bool CBDScModuleMgr::StopModule(UInt32 dwModuleId) 
{
	bool bRet=false;
	if(m_ScModuleList.empty()) return false;
	pthread_mutex_lock( &m_mtxScModuleList );
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwModuleId) {
			bRet=StopScModule(pScModule);
			break;
		}
	}
	pthread_mutex_unlock( &m_mtxScModuleList );
	return bRet;
}

//检查模块是否处于运行状态
bool CBDScModuleMgr::ModuleIsRunning(UInt32 dwModuleId)
{
	bool bRet=false;
	if(m_ScModuleList.empty()) return false;
	pthread_mutex_lock( &m_mtxScModuleList );
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwModuleId) {
			bRet=pScModule->bIsRunning;
			break;
		}
	}
	pthread_mutex_unlock( &m_mtxScModuleList );
	return bRet;
}

//开始（下发)任务
bool CBDScModuleMgr::StartModuleTask(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData)
{
	bool bRet=false;
	if(m_ScModuleList.empty()) return false;	
	pthread_mutex_lock( &m_mtxScModuleList );
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwModuleId) {
			if(pScModule->bIsRunning) {
				if(pScModule->pScModuleClass!=NULL) {
					bRet=pScModule->pScModuleClass->StartTask(pDataType,pData);	
				}
			}
			break;
		}
	}
	pthread_mutex_unlock( &m_mtxScModuleList );
	return bRet;
}

//停止（取消）任务
bool CBDScModuleMgr::StopModuleTask(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData)
{
    bool bRet=false;
	if(m_ScModuleList.empty()) return false;

	pthread_mutex_lock( &m_mtxScModuleList );
    BD_SCMODULE_LIST::iterator it;
    for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
        if(pScModule==NULL) continue;
        if(pScModule->dwModuleID==dwModuleId) {
            if(pScModule->bIsRunning) {
				if(pScModule->pScModuleClass!=NULL) {
					bRet=pScModule->pScModuleClass->StopTask(pDataType,pData);	
				}
			}
			break;
		}
	}
	pthread_mutex_unlock( &m_mtxScModuleList );
	return bRet;
}


//调用方下发数据
bool CBDScModuleMgr::SetModuleData(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData,UInt32 dwLength)
{
	bool bRet=false;
	if(m_ScModuleList.empty()) return false;
	pthread_mutex_lock( &m_mtxScModuleList );
	UInt32 dwRealModuleId=GetRealModuleId(dwModuleId);
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwRealModuleId) {
            pScModule->bIsRunning = pScModule->pScModuleClass->IsRunning();
			if(!pScModule->bIsRunning) { //策略在启动插件时下发
				if(pScModule->pScModuleClass!=NULL) {
					bRet=pScModule->pScModuleClass->SetData(pDataType,pData,dwLength);	
				}
			}
			break;
		}
	}
	pthread_mutex_unlock( &m_mtxScModuleList );
	return bRet;
}

//调用方获取数据
void * CBDScModuleMgr::GetModuleData(UInt32 dwModuleId,const PModIntfDataType pDataType,UInt32& dwRetLen)
{
	void * pData=NULL;
	if(m_ScModuleList.empty()) return NULL;
	pthread_mutex_lock( &m_mtxScModuleList );

	UInt32 dwRealModuleId=GetRealModuleId(dwModuleId);
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it) {
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwRealModuleId) {
			if(pScModule->bIsRunning) {
				if(pScModule->pScModuleClass!=NULL) {
					pData=pScModule->pScModuleClass->GetData(pDataType,dwRetLen);	
				}
			}
			break;
		}
	}

	pthread_mutex_unlock( &m_mtxScModuleList );
	return pData;
}

//调用方释放获取到的内存
void CBDScModuleMgr::FreeModuleData(UInt32 dwModuleId,void * pData)
{
	if(m_ScModuleList.empty()) return;
	pthread_mutex_lock( &m_mtxScModuleList );
	
	BD_SCMODULE_LIST::iterator it;
	for(it=m_ScModuleList.begin();it!=m_ScModuleList.end(); ++it)
	{
		bd_scmodule_t * pScModule=*it;
		if(pScModule==NULL) continue;
		if(pScModule->dwModuleID==dwModuleId) {
			if(pScModule->bIsRunning) {
				if(pScModule->pScModuleClass!=NULL) {
					pScModule->pScModuleClass->FreeData(pData);	
				}
			}
			break;
		}
	}

	pthread_mutex_unlock( &m_mtxScModuleList );
}
//启动模块加载
bool CBDScModuleMgr::Start(void)
{
    if(m_ScModuleList.size() > 0)
        return true;
    else
        return Load();
}
//清空模块list
void CBDScModuleMgr::Stop(void)
{
	Unload();
}
//获取模块id
UInt32 CBDScModuleMgr::GetRealModuleId(UInt32 dwSrcModuleId)
{
	UInt32 dwDstModuleId=0;
#if defined(_WIN32) || defined(_WIN64)
	 dwDstModuleId=dwSrcModuleId;	
#else
	if(dwSrcModuleId==(UInt32)SCMODULE_EXTRACCONNMON)//非法连接其他设备监控
		dwDstModuleId=(UInt32)SCMODULE_PSNETBHMON;//非法连接外网监控
	else
		dwDstModuleId=dwSrcModuleId;	
#endif
	return dwDstModuleId;
}
//获取模块状态
void CBDScModuleMgr::GetModuleStateString(string &strModuleState)
{
    strModuleState = "";
	pthread_mutex_lock( &m_mtxScModuleList );
    BD_SCMODULE_LIST::iterator iterTmp = m_ScModuleList.begin();
	for(;iterTmp != m_ScModuleList.end();iterTmp++) {
		char chTemp[50] = {0};
		//发送心跳信息前再次获取一下插件状态
		(*iterTmp)->bIsRunning = (*iterTmp)->pScModuleClass->IsRunning();
		sprintf(chTemp,"%u%s%d",(*iterTmp)->dwModuleID,strFieldSep.c_str(),(*iterTmp)->bIsRunning);
		strModuleState += chTemp;
		strModuleState += strRecordSep;
	}
	//去掉最后一个记录分隔符
	if (strModuleState.length() > 0)
        strModuleState = strModuleState.substr(0,strModuleState.length()-1);
	pthread_mutex_unlock( &m_mtxScModuleList );
}






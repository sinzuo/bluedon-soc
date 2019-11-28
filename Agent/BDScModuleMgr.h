
/** ************************************************
* COPYRIGHT NOTICE
* Copyright (c) 2017,      BLUEDON
* All rights reserved.
*
* @file    BDScModuleMgr.h
* @brief   传感器插件管理, 管理各个插件
*
*
* @version 5.1
* @author  xxx
* @date    YYYY年MM月DD日
*
* 修订说明：最初版本
* *************************************************/


#ifndef _H_BDSCMODULE_MGR
#define _H_BDSCMODULE_MGR

#include <string>
#include <list>
#include <cstring>
#include <cstdio>
#include "Poco/Mutex.h"
#include "Poco/SharedLibrary.h"
#include "../common/BDScModuleBase.h"
#include "../common/BDIcspModuleBase.h"



typedef struct tagRequestEx        // 业务请求结构定义
{
	tagRequestEx() {
		dwModuleID = 0;
		strPolicy  = "";
		dwType     = 0;
		memset(&dataType_T, 0, sizeof(dataType_T));
	}
    Poco::UInt32 dwModuleID;       // module id
	std::string  strPolicy;        // 策略串(由于策略串长度的不确定性，这里使用string)
    Poco::UInt32 dwType;           // 请求业务类型  
    modintf_datatype_t dataType_T; 
} bd_request_t;

typedef struct tagScModuleMgrEx     //监控模块管理类型定义
{
	Poco::UInt32 dwModuleID;        // 模块ＩＤ
	Poco::UInt16 wModuleVer;        // 模块版本
	string strModuleName;           // 模块名称
	bool bIsRunning;                // 运行状态
	CScModuleBase * pScModuleClass; // 模块实例
	Poco::SharedLibrary * pShlib;   // 模块管理库实例
} bd_scmodule_t;

typedef list<bd_scmodule_t *> BD_SCMODULE_LIST;



/**
 * @brief   传感器插件管理模块
 * @class   CBDScModuleMgr
 * @author
 * @date    2016年4月
 */
class CBDScModuleMgr:public CBdIcspModuleBase
{
public:
	CBDScModuleMgr();
	virtual ~CBDScModuleMgr();

	virtual void Init(void);
    virtual bool Start(void);//启动模块加载
    virtual void Stop(void);//清空模块list

	static CBDScModuleMgr * instance();
	static void exit();

	bool StartModule(UInt32 dwModuleId); //启动模块
	bool StopModule(UInt32 dwModuleId); //停止模块
	bool ModuleIsRunning(UInt32 dwModuleId);//检查模块是否处于运行状态

	bool StartModuleTask(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData);//开始（下发)任务
	bool StopModuleTask(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData);//停止（取消）任务
	bool SetModuleData(UInt32 dwModuleId,const PModIntfDataType pDataType,const void * pData,Poco::UInt32 dwLength);//调用方下发数据
	void * GetModuleData(UInt32 dwModuleId,const PModIntfDataType pDataType,Poco::UInt32& dwRetLen);//调用方获取数据
	void FreeModuleData(UInt32 dwModuleId,void * pData);//调用方释放获取到的内存

    void GetModuleStateString(string &strModuleState);//获取模块状态


protected:
    bool AddScModule(bd_scmodule_t * pScModule); //添加模块到list
    bool StartScModule(bd_scmodule_t * pScModule); //启动模块
    bool StopScModule(bd_scmodule_t * pScModule);//停止模块
	
    bool LoadModule(string &strModule, string &strConfig);//加载指定的模块到内存
    void UnLoadScModule(bd_scmodule_t * pScModule);//释放内存中指定的模块

    bool Load(void); //读取配置文件中加载模块
    void Unload(void);//清空模块list
private:
    UInt32 GetRealModuleId(UInt32 dwSrcModuleId);  //获取模块id
	
private:
	static Poco::FastMutex  m_InsMutex;
	static CBDScModuleMgr  * m_instance;
    BD_SCMODULE_LIST m_ScModuleList;  //模块list
	pthread_mutex_t m_mtxScModuleList; //模块list锁

    string strFieldSep;  //属性分隔符
    string strRecordSep; //策略分隔符
};


#endif //_H_BDSCMODULE_MGR












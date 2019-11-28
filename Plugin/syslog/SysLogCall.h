#ifndef _SYSLOG_CALL_H_
#define _SYSLOG_CALL_H_

#include "SysLogMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif
//创建，释放模块接口
extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_SYSLOG_CALL_H_


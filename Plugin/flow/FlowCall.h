#ifndef _FLOW_CALL_H_
#define _FLOW_CALL_H_

#include "FlowMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_SYSLOG_CALL_H_


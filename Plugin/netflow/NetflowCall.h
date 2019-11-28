#ifndef _NETFLOW_CALL_H_
#define _NETFLOW_CALL_H_

#include "NetflowMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_NETFLOW_CALL_H_


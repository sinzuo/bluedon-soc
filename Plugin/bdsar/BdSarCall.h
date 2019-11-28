#ifndef _BDSAR_CALL_
#define _BDSAR_CALL_

#include "BdSarMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_BDSAR_CALL_


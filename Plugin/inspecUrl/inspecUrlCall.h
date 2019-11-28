#ifndef _INSPECURL_CALL_
#define _INSPECURL_CALL_

#include "inspecUrlMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_INSPECURL_CALL_


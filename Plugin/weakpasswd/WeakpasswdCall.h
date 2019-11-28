#ifndef _WEAKPASSWD_CALL_
#define _WEAKPASSWD_CALL_

#include "WeakpasswdMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_WEAKPASSWD_CALL_


#ifndef _NMAPSCAN_CALL_
#define _NMAPSCAN_CALL_

#include "NmapscanMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_NMAPSCAN_CALL_


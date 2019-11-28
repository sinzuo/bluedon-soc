#ifndef _WMI_CALL_H_
#define _WMI_CALL_H_

#include "WMIMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_WMI_CALL_H_


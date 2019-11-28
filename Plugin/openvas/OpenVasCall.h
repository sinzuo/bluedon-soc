#ifndef _OPENVAS_CALL_
#define _OPENVAS_CALL_

#include "OpenVasMgr.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern  CScModuleBase*   CreateScModule(const string& strConfigName);
extern  void  FreeScModule(CScModuleBase* pScModuleBase);

#ifdef __cplusplus
}
#endif

#endif //_BDSAR_CALL_


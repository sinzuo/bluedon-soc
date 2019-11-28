#ifndef _DOCKER_CALL_
#define _DOCKER_CALL_

#include "DockerMgr.h"

#ifdef __cplusplus 
extern "C"
{
#endif

extern  CScModuleBase *  CreateScModule(const string &strConfigName);
extern  void  FreeScModule(CScModuleBase * pScModuleBase);

#ifdef __cplusplus 
}
#endif

#endif //_DOCKER_CALL_


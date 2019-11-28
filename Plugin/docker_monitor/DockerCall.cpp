
#include "DockerCall.h"


CScModuleBase * BD_CALLBACK_TYPE CreateScModule(const string &strConfigName)
{
    CScModuleBase * pScModuleBase=(CScModuleBase *)new CDockerMgr(strConfigName);
	return pScModuleBase;
}
void BD_CALLBACK_TYPE FreeScModule(CScModuleBase * pScModuleBase)
{
	if(pScModuleBase!=NULL) delete pScModuleBase;
}







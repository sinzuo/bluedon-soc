
#include "SysLogCall.h"

//新建模块
CScModuleBase * BD_CALLBACK_TYPE CreateScModule(const string &strConfigName)
{
	CScModuleBase * pScModuleBase=(CScModuleBase *)new CSysLogMgr(strConfigName); 
	return pScModuleBase;
}
//释放模块
void BD_CALLBACK_TYPE FreeScModule(CScModuleBase * pScModuleBase)
{
	if(pScModuleBase!=NULL) delete pScModuleBase;
}








#include "SysLogCall.h"

//�½�ģ��
CScModuleBase * BD_CALLBACK_TYPE CreateScModule(const string &strConfigName)
{
	CScModuleBase * pScModuleBase=(CScModuleBase *)new CSysLogMgr(strConfigName); 
	return pScModuleBase;
}
//�ͷ�ģ��
void BD_CALLBACK_TYPE FreeScModule(CScModuleBase * pScModuleBase)
{
	if(pScModuleBase!=NULL) delete pScModuleBase;
}







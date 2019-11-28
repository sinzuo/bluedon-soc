#ifndef _H_BDICSPMODULEBASE
#define _H_BDICSPMODULEBASE

#include "Poco/Types.h"
//模块基类
class CBdIcspModuleBase
{
public:
	CBdIcspModuleBase(void)
	{

	}
	virtual ~CBdIcspModuleBase(void)
	{

	}

	virtual void Init(void) = 0;
	virtual bool Start(void) = 0;
	virtual void Stop(void) = 0;
};

#endif //_H_BDICSPMODULEBASE

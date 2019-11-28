#ifndef _AGENT_SINZUO_OPTIONS_H

#include "common/BDScModuleBase.h"
#include "config/BDOptions.h"
//配置文件结构
typedef struct BDControllerConfigEx {
    char chLog4File[100];
    int nModuleId;
	char chModuleName[20];
	UInt16 wModuleVersion;
    char chRecordSep[2];        //记录分隔符
	char chFieldSep[2];         //字段分隔符	
	int nTaskNumPerThread;      //每个线程处理的任务数
}tag_controller_config_t;

#define  CONTROLLER_INFO_V(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("CONTROLLER"), str)
#define  CONTROLLER_DEBUG_V(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("CONTROLLER"), str)
#define  CONTROLLER_WARN_V(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("CONTROLLER"), str)
#define  CONTROLLER_ERROR_V(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("CONTROLLER"), str)

#define  CONTROLLER_INFO_S(str) LOG4CXX_INFO(log4cxx::Logger::getLogger("CONTROLLER"), #str)
#define  CONTROLLER_DEBUG_S(str) LOG4CXX_DEBUG(log4cxx::Logger::getLogger("CONTROLLER"), #str)
#define  CONTROLLER_WARN_S(str) LOG4CXX_WARN(log4cxx::Logger::getLogger("CONTROLLER"), #str)
#define  CONTROLLER_ERROR_S(str) LOG4CXX_ERROR(log4cxx::Logger::getLogger("CONTROLLER"), #str)

#endif //_AGENT_CHECK_OPTIONS_H_



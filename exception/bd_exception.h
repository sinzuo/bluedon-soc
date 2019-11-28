

#ifndef __BD_EXCEPTION_H__
#define __BD_EXCEPTION_H__

#include "common/bd_basic.h"
#include <exception>
#include <string>


/** 异常
 * 
 * 
 */
class BD_EXPORT_CLASS bd_exception : public std::exception
{
public:
/** 异常等级
 * 
 * 
 */
    enum E_LEVEL {
        L_OTHER = -1,   ///< 其他
        L_FATAL = 1,    ///< 致命错误
        L_ERROR,        ///< 一般错误
        L_WARN          ///< 警告
    };

/**
 * 析构函数
 */
    virtual ~bd_exception() throw();

/**
 * 获取异常等级
 * @param   无
 * @return  
 */
    virtual int getcode() const = 0;

/**
 * 获取异常等级
 * @param   无
 * @return  异常等级
 */
    virtual E_LEVEL getlevel() const = 0;
};

#endif // __BD_EXCEPTION_H__


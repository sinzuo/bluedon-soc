
#ifndef BD_COMMON_H_
#define BD_COMMON_H_

#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <algorithm>
#include "common/bd_basic.h"

#ifdef WIN32
    #pragma warning(disable:4996)
    #ifdef BD_COMMON_EXPORTS
        #define BD_COMMONDLL_EXPORTS __declspec(dllexport)
    #else
        #define BD_COMMONDLL_EXPORTS __declspec(dllimport)
    #endif // BD_COMMON_EXPORTS
#else
    #define BD_COMMONDLL_EXPORTS
#endif // WIN32

#if defined(NO_CLIENT_LONG_LONG)
    typedef unsigned long bd_ulonglong;
    typedef long bd_longlong;
#elif defined (__WIN__)
    typedef unsigned __int64 bd_ulonglong;
    typedef __int64 bd_longlong;
#else
    typedef unsigned long long bd_ulonglong;
    typedef long long bd_longlong;
#endif

//////////////////////////////////////////////////////////////////////////
// string function

/**
 * 将字符串转成大写
 * @param buff  待处理字符串[_in_out]
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_makeupper(std::string* buff);

/**
 * 将字符串转成小写
 * @param buff  待处理字符串[_in_out]
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_makelower(std::string* buff);

int BD_COMMONDLL_EXPORTS bd_comparenocase(const std::string& src, const std::string& dst);

/**
 * 去除字符串首尾的空字符及指定的字符
 * @param buff  待处理字符串[_in_out]
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_trim( std::string* buff );

/**
 * 去除字符串左边的空字符及指定的字符
 * @param buff  待处理字符串[_in_out]
 * @param ch    指定去除的字符[_in]，默认为' '
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_trimleft(std::string* buff, char ch = ' ');

/**
 * 去除字符串左边的空字符及包含在指定字符串中的字符
 * @param buff  待处理字符串
 * @param str   包含去除字符的指定的字符串
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_trimleft(std::string* buff, const std::string& str);

/**
 * 去除字符串右边的空字符及指定的字符
 * @param buff  待处理字符串[_in_out]
 * @param ch    指定去除的字符[_in]，默认为' '
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_trimright(std::string* buff, char ch = ' ');

/**
 * 去除字符串右边的空字符及包含在指定字符串中的字符
 * @param buff  待处理字符串[_in_out]
 * @param str   包含去除字符的指定的字符串[_in]
 * @return      无
 */
void BD_COMMONDLL_EXPORTS bd_trimright(std::string* buff, const std::string& str);

/** 将字符串以指定的分隔符分割成两个子串
 * 找出buff中str第一次出现的位置，以str为分隔符，分割成两个子串
 * @param buff  待处理字符串[_in]
 * @param sep   分隔符[_in]
 * @param sub1  分隔符前面的子串[_out]
 * @param sub2  分隔符后面的子串[_out]
 * @return
 *       1   分割后只有一个子串，sub2为空串
 *       2   分割后有两个子串
 */
int BD_COMMONDLL_EXPORTS bd_split(const std::string& buff, const char* sep, std::string* sub1, std::string* sub2); 

/**
 * 将字符串以指定的分隔符分割成子串，并存放于list中
 * @param buff  待处理字符串[_in]
 * @param sep   分隔符[_in]
 * @param sub1  分隔后的子串list[_out]
 * @return      无  
 */
void BD_COMMONDLL_EXPORTS bd_split(const std::string& buff, const std::string& sep, std::list<std::string>* listsplit);

/**
 * 将字符串以指定的分隔符分割成子串，并存放于vector中
 * @param buff  待处理字符串[_in]
 * @param sep   分隔符[_in]
 * @param sub1  分隔后的子串vector[_out]
 * @return      无  
 */
void BD_COMMONDLL_EXPORTS bd_split(const std::string& buff, const std::string& sep, std::vector<std::string>* listsplit);

/**
 * 删除字符串中指定的字符
 * @param buff  待处理字符串[_in_out]
 * @param ch    需删除的字符[_in]
 * @return      无  
 */
void BD_COMMONDLL_EXPORTS bd_erasechar(std::string* buff, const unsigned char ch);

/** 删除字符串中的backspace及其前面的一个字符
 * buff是原始的字符流，含有backspace，此函数实质上是获取backspace作用后的字符串
 * @param buf   待处理字符串[_in_out]
 * @return      无  
 */
void BD_COMMONDLL_EXPORTS bd_erasebschar(std::string* buff);

/** 
 * 字符串转换为整数，例如字符串"333"转换为数字333
 * @param str   待转换的字符串[_in]
 * @param num   转换后的整数[_out]
 * @return
 *       0      转换成功
 *       -1     转换失败
 */
int BD_COMMONDLL_EXPORTS bd_str2int(const std::string& str, int* num);

/** 
 * 字符串转换为无符号整数，例如字符串"333"转换为数字333
 * @param str   待转换的字符串[_in]
 * @param num   转换后的无符号整数[_out]
 * @return
 *       0      转换成功
 *       -1     转换失败
 */
int BD_COMMONDLL_EXPORTS bd_str2uint(const std::string& str, unsigned int* num);

/**
 * 字符串转换为64位长整数，例如字符串"333"转换为数字333
 * @param str   待转换的字符串[_in]
 * @param num   转换后的无符号64位长整数[_out]
 * @return
 *       0      转换成功
 *       -1     转换失败
 */
int BD_COMMONDLL_EXPORTS bd_str2longlong(const std::string& str, bd_longlong* num);

/**
 * 字符串转换为无符号64位长整数，例如字符串"333"转换为数字333
 * @param str   待转换的字符串[_in]
 * @param num   转换后的无符号64位长整数[_out]
 * @return
 *       0      转换成功
 *       -1     转换失败
 */
int BD_COMMONDLL_EXPORTS bd_str2ulonglong(const std::string& str, bd_ulonglong* num);

/** 
 * 字符串转换为double型数字，例如字符串"333.5"转换为数字333.5
 * @param str   待转换的字符串[_in]
 * @param num   转换后的double型数[_out]
 * @return
 *       0      转换成功
 *       -1     转换失败
 */
int BD_COMMONDLL_EXPORTS bd_str2double(const std::string& str, double* num);
 
/** 
 * 整数转换为字符串，例如数字48转换为字符串"48"
 * @param num    待转换的整数[_in]
 * @return       转换后的字符串
 */
std::string BD_COMMONDLL_EXPORTS bd_int2str(int num);

/** 
 * 无符号整数转换为字符串，例如数字48转换为字符串"48"
 * @param num    待转换的整数[_in]
 * @return       转换后的字符串
 */
std::string BD_COMMONDLL_EXPORTS bd_uint2str(unsigned int num);

/** 
 * 64位长整数转换为字符串，例如数字48转换为字符串"48"
 * @param num    待转换的整数[_in]
 * @return       转换后的字符串
 */
std::string BD_COMMONDLL_EXPORTS bd_longlong2str( bd_longlong num );

/** 
 * 无符号64位长整数转换为字符串，例如数字48转换为字符串"48"
 * @param num    待转换的整数[_in]
 * @return       转换后的字符串
 */
std::string BD_COMMONDLL_EXPORTS bd_ulonglong2str( bd_ulonglong num );

/** 
 * double型数转换为字符串，例如数字48.3转换为字符串"48.3"
 * @param num    待转换的double型数[_in]
 * @return       转换后的字符串
 */
std::string BD_COMMONDLL_EXPORTS bd_double2str(double num);

/** 
 * 16进制格式字符串转换为整数，例如字符串"0x33"转换为数字51
 * @param str    待转换的16进制格式字符串[_in]
 * @param num    转换后的整数[_out]
 * @return
 *       0      转换成功
 *       -1     转换失败
 */
int BD_COMMONDLL_EXPORTS bd_hexstr2int(const std::string& str, int* num);

/** 
 * 整数转换为16进制格式字符串，例如数字48转换为字符串"0x30"
 * @param num    待转换的整数[_in]
 * @return       转换后的字符串
 */
std::string BD_COMMONDLL_EXPORTS bd_int2hexstr(int num);


//////////////////////////////////////////////////////////////////////////
// date time
/**
 * 获取当前日期
 * @param    无
 * @return   当前日前字符串，格式为"YYYY-MM-DD"
 */
std::string BD_COMMONDLL_EXPORTS bd_getcurdate();

/**
 * 获取当前时间
 * @param    无
 * @return   当前日前字符串，格式为"YYYY-MM-DD hh:mm:ss"
 */
std::string BD_COMMONDLL_EXPORTS bd_getcurdatetime();

/**
 * 获取当前时间
 * @param curyear    当前年份[_out]
 * @param curmonth   当前月份[_out]
 * @param curmday    当月第几天[_out]
 * @param curhour    小时[_out]
 * @param curminute  分钟[_out]
 * @param curwday    星期几[_out]
 * @return   无
 */
void BD_COMMONDLL_EXPORTS bd_getcurdatetime(std::string* curyear,
                                         std::string* curmonth,
                                         std::string* curmday,
                                         std::string* curhour,
                                         std::string* curminute,
                                         std::string* curwday);

/**
 * 获取当前日期
 * @param    无
 * @return   当前日前字符串，格式为"YYYYMMDD"
 */
std::string BD_COMMONDLL_EXPORTS bd_getdate(time_t t);

/**
 * 获取当前时间
 * @param    无
 * @return   当前日前字符串，格式为"YYYY-MM-DD hh:mm:ss"
 */
std::string BD_COMMONDLL_EXPORTS bd_getdatetime(time_t t);

/**
 * 将16进制字符串转换为ANSI字符串，输入为"45 3A 5C D4 CB D3 C3 C8 ED BC "
 * @param    输入/输出
 * @return
 */
void HexStringToString(std::string &strHexSrc);
/**
 * remove_copy_if的函数因子
 * @param
 * @return
 */
bool HexStringToString_functor(char var);

#endif /* BD_COMMON_H_ */



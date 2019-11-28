/** ************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2011,      BLUEDON
 * All rights reserved.
 * 
 * @file    bd_common.h
 * @brief   公用函数声明
 * 字符串处理，字符串与数字的转换，日期和时间获取
 *
 * @version 1.0 
 * @author  潘建锋/开发二部
 * @date    2011年09月06日
 *
 * 修订说明：最初版本
 **************************************************/


#include "bd_common.h"

#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <limits>
#include <cstdarg>

#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_time.h"


void bd_makeupper(std::string* buff)
{
    std::string::iterator it = buff->begin();
    while (buff->end() != it) {
        *it = toupper(*it);
        ++it;
    }
}

void bd_makelower(std::string* buff)
{
    std::string::iterator it = buff->begin();
    while (buff->end() != it) {
        *it = tolower(*it);
        ++it;
    }
}

int bd_comparenocase(const std::string& src, const std::string& dst)
{
    std::string srctmp(src);
    std::string dsttmp(dst);
    bd_makeupper(&srctmp);
    bd_makeupper(&dsttmp);
    return srctmp.compare(dsttmp);
}

void bd_trim( std::string* buff )
{
    bd_trimleft( buff );
    bd_trimright( buff );
}

void bd_trimleft(std::string* buff, char ch)
{
    int trimLen = 0;
    
    for (std::string::iterator it = buff->begin(); buff->end() != it; ++it) {
        if ((* it > static_cast<char>(0x20)) && (ch != * it))
            break;
        else
            trimLen++;
    }

    * buff = buff->substr(trimLen);
}

void bd_trimleft(std::string* buff, const std::string& str)
{
    int trimLen = 0;

    for (std::string::iterator it = buff->begin(); buff->end() != it; ++it) {
        if ((* it > static_cast<char>(0x20)) && (str.find(*it) == std::string::npos))
            break;
        else
            trimLen++;
    }

    * buff = buff->substr(trimLen);
}

void bd_trimright(std::string* buff, char ch)
{
    int trimLen = 0;

    std::string::reverse_iterator it = buff->rbegin();

    while (buff->rend() != it) {
        if ((* it > static_cast<char>(0x20)) && (ch != * it))
            break;
        else
            trimLen++;

        ++it;
    }

    * buff = buff->substr(0, buff->length() - trimLen);
}

void bd_trimright(std::string* buff, const std::string& str)
{
    int trimLen = 0;

    std::string::reverse_iterator it = buff->rbegin();

    while (buff->rend() != it) {
        if ((* it > static_cast<char>(0x20)) && (str.find(*it) == std::string::npos))
            break;
        else
            trimLen++;

        ++it;
    }

    * buff = buff->substr(0, buff->length() - trimLen);
}

int bd_split(const std::string& buff, const char* sep, std::string* sub1, std::string* sub2)
{
    int pos = buff.find(sep);

    if(pos < 0) {
        * sub1 = buff;
        * sub2 = "";

        return 1;
    } else {
        * sub1 = buff.substr(0, pos);
        * sub2 = buff.substr(pos + strlen(sep));

        return 2;
    }
}

void bd_split(const std::string& buff,
              const std::string& sep,
              std::list<std::string>* listSplit)
{
    int start = 0;
    int end = 0;

    do {
        end = buff.find(sep, start);
        if(end < 0) {
            listSplit->push_back(buff.substr(start));
            return;
        } else {
            listSplit->push_back(buff.substr(start, end - start));
            start = end + sep.size(); 
        }

    } while(1);
}

void bd_split(const std::string& buff,
              const std::string& sep,
              std::vector<std::string>* listSplit)
{
    int start = 0;
    int end = 0;

    do {
        end = buff.find(sep, start);
        if(end < 0) {
            listSplit->push_back(buff.substr(start));
            return;
        } else {
            listSplit->push_back(buff.substr(start, end - start));
            start = end + sep.size(); 
        }

    } while(1);
}

void bd_erasechar(std::string* buff, const unsigned char ch)
{
    std::string::iterator it = buff->begin();
    while(buff->end() != it) {
        if(ch == (unsigned char)*it)
            it = buff->erase(it);
        else
            ++it;
    }
}

void bd_erasebschar(std::string* buff)
{
    std::string tmpBuff;

    std::string::iterator it = buff->begin();
    while(it != buff->end()) {
        if(8 == *it) {
            if(!tmpBuff.empty())
                tmpBuff = tmpBuff.substr(0, tmpBuff.length() - 1);
        } else {
            tmpBuff.push_back(*it);
        }

        ++it;
    }

    * buff = tmpBuff;
}

int bd_vsnprintf(char* str, size_t size, const char* fmt, ...)
{
    va_list vaList;
    va_start(vaList, fmt);

#ifdef WIN32
    _vsnprintf_s(str, size, _TRUNCATE, fmt, vaList);
#else
    vsnprintf(str, size, fmt, vaList);
#endif

    va_end(vaList);

    return 0;
}

int bd_str2int(const std::string& str, int* num)
{
    if (str.empty())
    {
        *num = 0;
        return 0;
    }

    std::stringstream ss;
    ss << str;
    ss >> * num;

    if (ss.fail())
        return -1;
    return 0;
}

int bd_str2uint(const std::string& str, unsigned int* num)
{
    if (str.empty())
    {
        *num = 0U;
        return 0;
    }

    std::stringstream ss;
    ss << str;
    ss >> * num;

    if (ss.fail())
        return -1;

    return 0;
}

int bd_str2longlong(const std::string& str, bd_longlong* num)
{
    if (str.empty())
    {
        *num = 0L;
        return 0;
    }

    std::stringstream ss;
    ss << str;
    ss >> * num;

    if (ss.fail())
        return -1;

    return 0;
}

int bd_str2ulonglong(const std::string& str, bd_ulonglong* num)
{
    if (str.empty())
    {
        *num = 0UL;
        return 0;
    }

    std::stringstream ss;
    ss << str;
    ss >> * num;

    if (ss.fail())
        return -1;

    return 0;
}

int bd_str2double(const std::string& str, double* num)
{
    if (str.empty())
    {
        *num = 0.0;
        return 0;
    }

    std::stringstream ss;
    ss.precision(std::numeric_limits<double>::digits10);
    ss << str;
    ss >> * num;

    if (ss.fail())
        return -1;

    return 0;
}

std::string bd_int2str(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string bd_uint2str(unsigned int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string bd_longlong2str( bd_longlong num )
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string bd_ulonglong2str( bd_ulonglong num )
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string bd_double2str(double num)
{
    std::stringstream ss;
    ss.precision(std::numeric_limits<double>::digits10);
    ss << num;
    return ss.str();
}

int bd_hexstr2int(const std::string& str, int* num)
{
    std::stringstream ss;
    ss.flags(std::ios::hex | std::ios::showbase);
    ss << str;
    ss >> * num;

    if (ss.fail())
        return -1;
    return 0;
}

std::string bd_int2hexstr(int num)
{
    std::string retVal;
    std::stringstream ss;
    ss.flags(std::ios::hex | std::ios::showbase);
    ss << num;
    ss >> retVal;
    return retVal;
}

std::string bd_getcurdate()
{
    time_t t;
    ACE_OS::time(&t);
    return bd_getdate(t);
}

std::string bd_getcurdatetime()
{
    time_t t;
    ACE_OS::time(&t);
    return bd_getdatetime(t);
}

void bd_getcurdatetime(std::string* curYear,
                       std::string* curMonth,
                       std::string* curMDay,
                       std::string* curHour,
                       std::string* curMinute,
                       std::string* curWDay)
{
    char tmp[32] = {0};

    time_t t;
    tm tv;

    ACE_OS::time(&t);
    ACE_OS::localtime_r(&t, &tv);

    ACE_OS::snprintf(tmp, 32, "%04d", tv.tm_year + 1900);
    * curYear = tmp;
    ACE_OS::snprintf(tmp, 32, "%02d", tv.tm_mon + 1);
    * curMonth = tmp;
    ACE_OS::snprintf(tmp, 32, "%02d", tv.tm_mday);
    * curMDay = tmp;
    ACE_OS::snprintf(tmp, 32, "%02d", tv.tm_hour);
    * curHour = tmp;
    ACE_OS::snprintf(tmp, 32, "%02d", tv.tm_min);
    * curMinute = tmp;
    ACE_OS::snprintf(tmp, 32, "%02d", tv.tm_wday);
    * curWDay = tmp;
}

std::string bd_getdate(time_t t)
{
    char tmp[20] = {0, };

    tm tv;
    ACE_OS::localtime_r(&t, &tv);

    ACE_OS::snprintf(tmp, 20,
        "%04d%02d%02d",
        tv.tm_year + 1900,
        tv.tm_mon + 1,
        tv.tm_mday);

    return std::string(tmp);	
}

std::string bd_getdatetime(time_t t)
{
    char tmp[32] = {0, };

    tm tv;
    ACE_OS::localtime_r(&t, &tv);

    ACE_OS::snprintf(tmp, 32, 
        "%04d-%02d-%02d %02d:%02d:%02d",
        tv.tm_year + 1900,
        tv.tm_mon + 1,
        tv.tm_mday,
        tv.tm_hour,
        tv.tm_min,
        tv.tm_sec);

    return std::string(tmp);
}

bool HexStringToString_functor(char val) {
    return (val == ' ')||(val == '\n')||(val == '\r')||(val == '\t')||(val == '\f');
}

void HexStringToString(std::string &strHexSrc) {
	int nlen = strHexSrc.length();
	if (nlen == 0)
		return;
	char chTmp[512] = { 0 };
	//remove_copy(strHexSrc.begin(), strHexSrc.end(), chTmp, ' ');
    remove_copy_if(strHexSrc.begin(), strHexSrc.end(), chTmp, HexStringToString_functor);
	if (strlen(chTmp)%2 != 0 ) return ; // 删除 无效字符就为偶数了
	std::string strTmp = chTmp;
	std::stringstream ss_out;
	for (int i = 0; i < strTmp.length(); i += 2) {
		std::stringstream ss_in;
		ss_in.flags(std::ios::hex | std::ios::showbase);
		ss_in << strTmp.substr(i, 2);
		unsigned int a;
		ss_in >> a;
		ss_out << (unsigned char) (a);
	}
	if (ss_out.str().length() > 0)
		strHexSrc = ss_out.str();
}

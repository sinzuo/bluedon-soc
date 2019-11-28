//#include "../common/stdafx.h"
#include "bdstring.h"

#if defined(POCO_OS_FAMILY_WINDOWS)
	#include "Poco/UnWindows.h"
#endif
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
#include "Poco/SignalHandler.h"
#endif
#if defined(POCO_WIN32_UTF8) && !defined(POCO_NO_WSTRING)
#include "Poco/UnicodeConverter.h"
#endif

#include "Poco/UnicodeConverter.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/UTF16Encoding.h"
#include "Poco/ASCIIEncoding.h"
#include "Poco/TextEncoding.h"
#include "Poco/TextConverter.h"
#include <stdio.h>
#include <iostream>

using Poco::Path;
using Poco::PathSyntaxException;
using Poco::Environment;
using Poco::SystemException;
using namespace std;

namespace BDSTRING
{
void getAppPath(Poco::Path& appPath,std::string _command) 
{
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
	std::string _workingDirAtLaunch=Path::current();
        if (_command.find('/') != std::string::npos)
	{
		Path path(_command);
		if (path.isAbsolute())
		{
			appPath = path;
		}
		else
		{
			appPath = _workingDirAtLaunch;
			appPath.append(path);
		}
	}
	else
	{
		if (!Path::find(Environment::get("PATH"), _command, appPath))
			appPath = Path(_workingDirAtLaunch, _command);
		appPath.makeAbsolute();
	}
#elif defined(POCO_OS_FAMILY_WINDOWS)
	#if defined(POCO_WIN32_UTF8) && !defined(POCO_NO_WSTRING)
		wchar_t path[1024];
		int n = GetModuleFileNameW(0, path, sizeof(path)/sizeof(wchar_t));
		if (n > 0)
		{
			std::string p;
			Poco::UnicodeConverter::toUTF8(path, p);
			appPath = p;
		}
		else throw SystemException("Cannot get application file name.");
	#else
		char path[1024];
		int n = GetModuleFileNameA(0, path, sizeof(path));
		if (n > 0)
			appPath = path;
		else
			throw SystemException("Cannot get application file name.");
	#endif
#else
	appPath = _command;
#endif
}

void getSubPath(const Poco::Path parentPath,const std::string strSubPath,Poco::Path& subPath)
{
	std::string strParentPath=parentPath.toString();
	
	strParentPath += strSubPath;
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
	subPath=Path(strParentPath, Path::PATH_UNIX);
#elif defined(POCO_OS_FAMILY_WINDOWS)
	subPath=Path(strParentPath, Path::PATH_WINDOWS);
#endif
}

size_t strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/** Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/** Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';      /** NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);    /** count does not include NUL */
}

size_t strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/** Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));   /** count does not include NUL */
}

void strtrim(char * szBuf)
{
	if(szBuf==NULL || szBuf[0]==0x00)
		return;
	char * pStr=szBuf;
	for(size_t n=0;n<strlen(szBuf);n++)
	{
		if(szBuf[n]==0x20)
			pStr++;
		else 
			break;
	}
	if(pStr!=szBuf)
	{
		size_t nLength=strlen(szBuf)-(pStr-szBuf);
		for(unsigned int n=0;n<nLength;n++) 
		{
			*(szBuf+n)=*(pStr+n); 
		}
		*(szBuf+nLength)=0x00;
	}
	for(size_t  n=strlen(szBuf)-1;n>0;n--)
	{
		if(szBuf[n]==0x20 || szBuf[n]==0x0a || szBuf[n]==0x0d)
			szBuf[n]=0x00;
		else
			break;
	}
}


char * strrchr (register const char *s, int c)
{
	char *rtnval = 0;

	do  
	{
		if (*s == c)
			rtnval = (char*) s;
	} while (*s++);
	return (rtnval);
}

void strtoupper(char * szBuf)
{
	if(szBuf==NULL || szBuf[0]==0x00)
		return;
	size_t nLength=(size_t)strlen(szBuf);
	for(size_t n=0;n<nLength;n++)
	{
		if(szBuf[n]>='a' && szBuf[n]<='z')
			szBuf[n]=toupper(szBuf[n]); 
	}
}


/*
void A2UTF8(const std::string asciiString, std::string & utf8String)
{
	utf8String.clear();
	if(asciiString.length()<=0)
		return;
	Poco::ASCIIEncoding asciiEncoding;
	Poco::UTF8Encoding utf8Encoding;
	Poco::TextConverter converter(asciiEncoding, utf8Encoding);
	converter.convert(asciiString.data(), (int) asciiString.length()*sizeof(char), utf8String);
}
*/

#if defined(POCO_OS_FAMILY_WINDOWS)
void A2UTF8(const std::string asciiString, std::string & utf8String)
{
	if(asciiString.length()<=0)
	{
		utf8String="";
		return;
	}
	int len=MultiByteToWideChar(CP_ACP, 0, asciiString.c_str(), -1, NULL,0);
	unsigned short * wszUtf8 = new unsigned short[len+1];
	if(wszUtf8==NULL)
	{
		utf8String="";
		return;
	}
	memset(wszUtf8, 0, len * 2 + 2);
	MultiByteToWideChar(CP_ACP, 0, asciiString.c_str(), -1, (LPWSTR)wszUtf8, len);

	len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, NULL, 0, NULL, NULL);
	char *szUtf8=new char[len + 1];
	if(szUtf8==NULL)
	{
		delete[] wszUtf8;
		utf8String="";
		return;
	}
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte (CP_UTF8, 0, (LPCWSTR)wszUtf8, -1, szUtf8, len, NULL,NULL);

	utf8String = szUtf8;

	if(szUtf8)
		delete[] szUtf8;
	if(wszUtf8)
		delete[] wszUtf8;
}

void UTF82A(const std::string utf8String, std::string & asciiString)
{
	asciiString.clear();
	if(utf8String.length()<=0)
		return;
	int len=MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)utf8String.c_str(), -1, NULL,0);
	unsigned short * wszGBK = new unsigned short[len+1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)utf8String.c_str(), -1, (LPWSTR)wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK=new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte (CP_ACP, 0, (LPCWSTR)wszGBK, -1, szGBK, len, NULL,NULL);

	asciiString = szGBK;

	if(szGBK)
		delete[] szGBK;
	if(wszGBK)
		delete[] wszGBK;
}
#else //linux

#include <stdio.h>
#include <iconv.h>
#include <string.h>

void A2UTF8(const std::string asciiString,std::string & utf8String)
{
	iconv_t    ivc  = (iconv_t )-1;

	size_t nSrcLen=asciiString.length();
/*
	char * pSrcBuf=(char *)malloc((nSrcLen+2)*sizeof(char));
	if(pSrcBuf==NULL)
		return;
	strncpy(pSrcBuf,asciiString.c_str(),nSrcLen);
	*(pSrcBuf+nSrcLen)=0x00;
*/
	if(nSrcLen<=0)
	{
		utf8String="";
		return;
	}

	ivc = iconv_open("UTF-8","GBK");
	if (ivc < 0)
	{
		utf8String="";
		printf("A2UTF8::iconv_open failed!\n");
		return;
	}
	size_t nDstLen=nSrcLen*4;
	char *pDstBuf=(char *)malloc((nDstLen+2)*sizeof(char));

	if (pDstBuf==NULL)
	{
		utf8String="";
		iconv_close( ivc );
		return;
	}
	memset(pDstBuf,0x00, nDstLen+2);
	size_t outlen = nDstLen;
	const char * inbuf=asciiString.c_str();
	char* outbuf = pDstBuf;
	
	int nRet=iconv(ivc,  (char **)&inbuf,(size_t *) &nSrcLen, (char **)&outbuf,(size_t *)&outlen); 
	//printf("nRet=%d\n",nRet);
	if(nRet<0)
	{
		free(pDstBuf);
		utf8String="";
		iconv_close( ivc );
		printf("A2UTF8::iconv failed!");
		//printf("iconv from UTF to GBK error: %s\n", strerror(errno));
		return;
	}
	utf8String=outbuf - (nDstLen - outlen);
	free(pDstBuf);
	iconv_close(ivc);
}

void UTF82A(const string utf8String, string & asciiString)
{
	iconv_t    ivc  = (iconv_t )-1;

	size_t nSrcLen=utf8String.length();
	if(nSrcLen<=0)
	{
		asciiString="";
		return;
	}

	ivc = iconv_open("GBK","UTF-8");
	if (ivc < 0)
	{
		asciiString="";
		return;
	}
	size_t nDstLen=nSrcLen*4;
	char *pDstBuf=(char*)malloc((nDstLen+2)*sizeof(char));
	
	if (pDstBuf==NULL)
	{
		asciiString="";
		iconv_close( ivc );
		return;
	}
	memset(pDstBuf, 0x00, nDstLen+2);
	const char * pSrcBuf=utf8String.c_str();
	char * outbuf=pDstBuf;
	size_t outlen=nDstLen;

	if (iconv(ivc,  (char **)&pSrcBuf, &nSrcLen, &outbuf, &outlen)<0) 
	{
		free(pDstBuf);
		asciiString="";
		iconv_close( ivc );
		return;
	}
	asciiString=outbuf - (nDstLen - outlen);
	free(pDstBuf);

	iconv_close(ivc);
}

#endif //defined(POCO_OS_FAMILY_WINDOWS)


}//namespace


//修改使函数支持字符串进行分割, 同时支持传入分割符为char *; 2017-12-14
void string_split(string &strsrc, string strpattern, vector<string> &vec, bool btrim)
{
    string::size_type newpos = 0;
	string::size_type oldpos = 0;
    size_t pat_len = strpattern.length();
	string str;
    while(true) {	
	    newpos = strsrc.find(strpattern, oldpos);
	    if(newpos == strsrc.npos) break;
        str = strsrc.substr(oldpos, newpos-oldpos);
        if( !str.empty() || !btrim ) vec.push_back(str);
        oldpos = newpos + pat_len;
	}
    str = strsrc.substr(oldpos);
    if( !str.empty() || !btrim ) vec.push_back(str);
}


// 分~时~日~月~周 (默认字段则填 "*" )
bool check_cron(string &str_crontab) { 
    vector<string> vec_temp;
	string strFSep = "~";
    string_split(str_crontab, strFSep, vec_temp);
	if (vec_temp.size() != 5 ) return false;

    time_t t = time(NULL);
    struct tm now;
    localtime_r(&t, &now);
    if( vec_temp[4].compare("*") != 0) {
		if ( now.tm_wday == 0 ) now.tm_wday = 7;// 星期天
	    if ( ((atoi(vec_temp[4].c_str()))) >> now.tm_wday-1  %2 == 0  ) return false;
    }
    if( vec_temp[3].compare("*") != 0) {  //前台不支持选择多个月
	    if (atoi(vec_temp[3].c_str()) != now.tm_mon+1) return false;
    }
    if( vec_temp[2].compare("*") != 0) { 
	    if (((atoi(vec_temp[2].c_str()))) >> now.tm_mday -1   %2 == 0) return false;
    }
    if( vec_temp[1].compare("*") != 0) { //前台不支持选多个时间点(时)
	    if (atoi(vec_temp[1].c_str()) != now.tm_hour) return false;
    }
    if( vec_temp[0].compare("*") != 0) { //前台不支持选多个时间点(分)
		if (atoi(vec_temp[0].c_str()) != now.tm_min) return false;
    }
	
	return true;
}





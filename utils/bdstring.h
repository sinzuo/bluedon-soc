#ifndef _H_BDSTRING
#define _H_BDSTRING

#include "Poco/Types.h"
#include "Poco/Environment.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/String.h"

#include <string>
#include <vector>
using namespace std;

namespace BDSTRING
{
extern void getAppPath(Poco::Path& appPath,std::string _command="");
extern void getSubPath(const Poco::Path parentPath,const std::string strSubPath,Poco::Path& subPath);
extern size_t strlcpy(char *dst, const char *src, size_t siz);
extern size_t strlcat(char *dst, const char *src, size_t siz);
extern void strtrim(char * szBuf);
extern char * strrchr (register const char *s, int c);
extern void strtoupper(char * szBuf);
extern void A2UTF8(const std::string asciiString, std::string & utf8String);
extern void UTF82A(const std::string utf8String, std::string & asciiString);
}


void string_split(string &strsrc, string strpattern, vector<string> &vec, bool btrim = true);
bool check_cron(string &str_crontab); 


#endif //_H_BDSTRING



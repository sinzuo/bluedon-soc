
#ifndef __BD_BASIC_H__
#define __BD_BASIC_H__

#ifdef WIN32
#define __putenv _putenv
#define __snprintf sprintf_s
#define __strncpy strcpy_s
#define __strncat strcat_s
#define __strdup _strdup
#else
#define __putenv putenv
#define __snprintf snprintf
#define __strncpy(dest, size_d, src) strncpy(dest, src, size_d)
#define __strncat(dest, size_d, src) strncat(dest, src, size_d)
#define __strdup strdup
#endif

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

#ifdef WIN32
    #pragma warning(disable:4996)
//    #ifdef BD_BUILD_DLL
        #define BD_EXPORT_CLASS __declspec(dllexport)   ///< 类
        #define BD_EXPORT_API   __declspec(dllexport)   ///< 函数
        #define BD_EXPORT_DATA  __declspec(dllexport)   ///< 变量
//    #else
//        #define BD_EXPORT_CLASS __declspec(dllimport)
//        #define BD_EXPORT_API   __declspec(dllimport)
//        #define BD_EXPORT_DATA  __declspec(dllimport)
//    #endif // BD_BUILD_DLL
#else
    #define BD_EXPORT_CLASS
    #define BD_EXPORT_API
    #define BD_EXPORT_DATA
#endif // WIN32

#endif // __BD_BASIC_H__

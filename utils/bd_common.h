
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
 * ���ַ���ת�ɴ�д
 * @param buff  �������ַ���[_in_out]
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_makeupper(std::string* buff);

/**
 * ���ַ���ת��Сд
 * @param buff  �������ַ���[_in_out]
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_makelower(std::string* buff);

int BD_COMMONDLL_EXPORTS bd_comparenocase(const std::string& src, const std::string& dst);

/**
 * ȥ���ַ�����β�Ŀ��ַ���ָ�����ַ�
 * @param buff  �������ַ���[_in_out]
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_trim( std::string* buff );

/**
 * ȥ���ַ�����ߵĿ��ַ���ָ�����ַ�
 * @param buff  �������ַ���[_in_out]
 * @param ch    ָ��ȥ�����ַ�[_in]��Ĭ��Ϊ' '
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_trimleft(std::string* buff, char ch = ' ');

/**
 * ȥ���ַ�����ߵĿ��ַ���������ָ���ַ����е��ַ�
 * @param buff  �������ַ���
 * @param str   ����ȥ���ַ���ָ�����ַ���
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_trimleft(std::string* buff, const std::string& str);

/**
 * ȥ���ַ����ұߵĿ��ַ���ָ�����ַ�
 * @param buff  �������ַ���[_in_out]
 * @param ch    ָ��ȥ�����ַ�[_in]��Ĭ��Ϊ' '
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_trimright(std::string* buff, char ch = ' ');

/**
 * ȥ���ַ����ұߵĿ��ַ���������ָ���ַ����е��ַ�
 * @param buff  �������ַ���[_in_out]
 * @param str   ����ȥ���ַ���ָ�����ַ���[_in]
 * @return      ��
 */
void BD_COMMONDLL_EXPORTS bd_trimright(std::string* buff, const std::string& str);

/** ���ַ�����ָ���ķָ����ָ�������Ӵ�
 * �ҳ�buff��str��һ�γ��ֵ�λ�ã���strΪ�ָ������ָ�������Ӵ�
 * @param buff  �������ַ���[_in]
 * @param sep   �ָ���[_in]
 * @param sub1  �ָ���ǰ����Ӵ�[_out]
 * @param sub2  �ָ���������Ӵ�[_out]
 * @return
 *       1   �ָ��ֻ��һ���Ӵ���sub2Ϊ�մ�
 *       2   �ָ���������Ӵ�
 */
int BD_COMMONDLL_EXPORTS bd_split(const std::string& buff, const char* sep, std::string* sub1, std::string* sub2); 

/**
 * ���ַ�����ָ���ķָ����ָ���Ӵ����������list��
 * @param buff  �������ַ���[_in]
 * @param sep   �ָ���[_in]
 * @param sub1  �ָ�����Ӵ�list[_out]
 * @return      ��  
 */
void BD_COMMONDLL_EXPORTS bd_split(const std::string& buff, const std::string& sep, std::list<std::string>* listsplit);

/**
 * ���ַ�����ָ���ķָ����ָ���Ӵ����������vector��
 * @param buff  �������ַ���[_in]
 * @param sep   �ָ���[_in]
 * @param sub1  �ָ�����Ӵ�vector[_out]
 * @return      ��  
 */
void BD_COMMONDLL_EXPORTS bd_split(const std::string& buff, const std::string& sep, std::vector<std::string>* listsplit);

/**
 * ɾ���ַ�����ָ�����ַ�
 * @param buff  �������ַ���[_in_out]
 * @param ch    ��ɾ�����ַ�[_in]
 * @return      ��  
 */
void BD_COMMONDLL_EXPORTS bd_erasechar(std::string* buff, const unsigned char ch);

/** ɾ���ַ����е�backspace����ǰ���һ���ַ�
 * buff��ԭʼ���ַ���������backspace���˺���ʵ�����ǻ�ȡbackspace���ú���ַ���
 * @param buf   �������ַ���[_in_out]
 * @return      ��  
 */
void BD_COMMONDLL_EXPORTS bd_erasebschar(std::string* buff);

/** 
 * �ַ���ת��Ϊ�����������ַ���"333"ת��Ϊ����333
 * @param str   ��ת�����ַ���[_in]
 * @param num   ת���������[_out]
 * @return
 *       0      ת���ɹ�
 *       -1     ת��ʧ��
 */
int BD_COMMONDLL_EXPORTS bd_str2int(const std::string& str, int* num);

/** 
 * �ַ���ת��Ϊ�޷��������������ַ���"333"ת��Ϊ����333
 * @param str   ��ת�����ַ���[_in]
 * @param num   ת������޷�������[_out]
 * @return
 *       0      ת���ɹ�
 *       -1     ת��ʧ��
 */
int BD_COMMONDLL_EXPORTS bd_str2uint(const std::string& str, unsigned int* num);

/**
 * �ַ���ת��Ϊ64λ�������������ַ���"333"ת��Ϊ����333
 * @param str   ��ת�����ַ���[_in]
 * @param num   ת������޷���64λ������[_out]
 * @return
 *       0      ת���ɹ�
 *       -1     ת��ʧ��
 */
int BD_COMMONDLL_EXPORTS bd_str2longlong(const std::string& str, bd_longlong* num);

/**
 * �ַ���ת��Ϊ�޷���64λ�������������ַ���"333"ת��Ϊ����333
 * @param str   ��ת�����ַ���[_in]
 * @param num   ת������޷���64λ������[_out]
 * @return
 *       0      ת���ɹ�
 *       -1     ת��ʧ��
 */
int BD_COMMONDLL_EXPORTS bd_str2ulonglong(const std::string& str, bd_ulonglong* num);

/** 
 * �ַ���ת��Ϊdouble�����֣������ַ���"333.5"ת��Ϊ����333.5
 * @param str   ��ת�����ַ���[_in]
 * @param num   ת�����double����[_out]
 * @return
 *       0      ת���ɹ�
 *       -1     ת��ʧ��
 */
int BD_COMMONDLL_EXPORTS bd_str2double(const std::string& str, double* num);
 
/** 
 * ����ת��Ϊ�ַ�������������48ת��Ϊ�ַ���"48"
 * @param num    ��ת��������[_in]
 * @return       ת������ַ���
 */
std::string BD_COMMONDLL_EXPORTS bd_int2str(int num);

/** 
 * �޷�������ת��Ϊ�ַ�������������48ת��Ϊ�ַ���"48"
 * @param num    ��ת��������[_in]
 * @return       ת������ַ���
 */
std::string BD_COMMONDLL_EXPORTS bd_uint2str(unsigned int num);

/** 
 * 64λ������ת��Ϊ�ַ�������������48ת��Ϊ�ַ���"48"
 * @param num    ��ת��������[_in]
 * @return       ת������ַ���
 */
std::string BD_COMMONDLL_EXPORTS bd_longlong2str( bd_longlong num );

/** 
 * �޷���64λ������ת��Ϊ�ַ�������������48ת��Ϊ�ַ���"48"
 * @param num    ��ת��������[_in]
 * @return       ת������ַ���
 */
std::string BD_COMMONDLL_EXPORTS bd_ulonglong2str( bd_ulonglong num );

/** 
 * double����ת��Ϊ�ַ�������������48.3ת��Ϊ�ַ���"48.3"
 * @param num    ��ת����double����[_in]
 * @return       ת������ַ���
 */
std::string BD_COMMONDLL_EXPORTS bd_double2str(double num);

/** 
 * 16���Ƹ�ʽ�ַ���ת��Ϊ�����������ַ���"0x33"ת��Ϊ����51
 * @param str    ��ת����16���Ƹ�ʽ�ַ���[_in]
 * @param num    ת���������[_out]
 * @return
 *       0      ת���ɹ�
 *       -1     ת��ʧ��
 */
int BD_COMMONDLL_EXPORTS bd_hexstr2int(const std::string& str, int* num);

/** 
 * ����ת��Ϊ16���Ƹ�ʽ�ַ�������������48ת��Ϊ�ַ���"0x30"
 * @param num    ��ת��������[_in]
 * @return       ת������ַ���
 */
std::string BD_COMMONDLL_EXPORTS bd_int2hexstr(int num);


//////////////////////////////////////////////////////////////////////////
// date time
/**
 * ��ȡ��ǰ����
 * @param    ��
 * @return   ��ǰ��ǰ�ַ�������ʽΪ"YYYY-MM-DD"
 */
std::string BD_COMMONDLL_EXPORTS bd_getcurdate();

/**
 * ��ȡ��ǰʱ��
 * @param    ��
 * @return   ��ǰ��ǰ�ַ�������ʽΪ"YYYY-MM-DD hh:mm:ss"
 */
std::string BD_COMMONDLL_EXPORTS bd_getcurdatetime();

/**
 * ��ȡ��ǰʱ��
 * @param curyear    ��ǰ���[_out]
 * @param curmonth   ��ǰ�·�[_out]
 * @param curmday    ���µڼ���[_out]
 * @param curhour    Сʱ[_out]
 * @param curminute  ����[_out]
 * @param curwday    ���ڼ�[_out]
 * @return   ��
 */
void BD_COMMONDLL_EXPORTS bd_getcurdatetime(std::string* curyear,
                                         std::string* curmonth,
                                         std::string* curmday,
                                         std::string* curhour,
                                         std::string* curminute,
                                         std::string* curwday);

/**
 * ��ȡ��ǰ����
 * @param    ��
 * @return   ��ǰ��ǰ�ַ�������ʽΪ"YYYYMMDD"
 */
std::string BD_COMMONDLL_EXPORTS bd_getdate(time_t t);

/**
 * ��ȡ��ǰʱ��
 * @param    ��
 * @return   ��ǰ��ǰ�ַ�������ʽΪ"YYYY-MM-DD hh:mm:ss"
 */
std::string BD_COMMONDLL_EXPORTS bd_getdatetime(time_t t);

/**
 * ��16�����ַ���ת��ΪANSI�ַ���������Ϊ"45 3A 5C D4 CB D3 C3 C8 ED BC "
 * @param    ����/���
 * @return
 */
void HexStringToString(std::string &strHexSrc);
/**
 * remove_copy_if�ĺ�������
 * @param
 * @return
 */
bool HexStringToString_functor(char var);

#endif /* BD_COMMON_H_ */



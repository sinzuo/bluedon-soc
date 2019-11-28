
#ifndef __BD_CRYPTO_MD5_H__
#define __BD_CRYPTO_MD5_H__

#include <string>
#include "common/bd_basic.h"

/** md5摘要
 * 
 * md5摘要
 */
class BD_EXPORT_CLASS bd_crypto_md5
{
public:
    /**
     * 对字符串内容做md5摘要
     * @param in    需校验的字符串指针[_in]
     * @param inLen 字符串长度[_in]
     * @param out   long out[4]md5值[_out]
     * @return      无
     */
    static void encodeUL4(const char* in, unsigned long inLen, unsigned long out[4]); 

    /**
     * 对字符串内容做md5摘要
     * @param in    需校验的字符串指针[_in]
     * @param inLen 字符串长度[_in]
     * @param out   16字节的md5值[_out]
     * @return      无
     */
    static void encodeUC16(const char* in, unsigned long inLen, unsigned char out[16]); 

    /**
     * 对字符串内容做md5摘要
     * @param in    需校验的字符串指针[_in]
     * @param inLen 字符串长度[_in]
     * @return      string型的md5值
     */
    static std::string encodeUC16(const char* in, unsigned long inLen);

    /**
     * 对字符串内容做md5摘要
     * @param in    需校验的字符串指针[_in]
     * @param inLen 字符串长度[_in]
     * @param out   16字节的md5值[_out]
     * @return      无
     */
	static void encodeUC32(const char* in, unsigned long inLen, unsigned char out[32]);

    /**
     * 对字符串内容做md5摘要
     * @param in    需校验的字符串指针[_in]
     * @param inLen 字符串长度[_in]
     * @return      string型的md5值
     */
	static std::string encodeUC32(const char* in, unsigned long inLen);

    /**
     * 对文件内容做md5摘要
     * @param in    需校验的文件[_in]
     * @param out   16字节的md5值[_out]
     * @return 
     *      ture    成功
     *      false   失败
     */
    static bool encodeFileUC16( const std::string fileName, unsigned char out[16] );
};

#endif // __BD_CRYPTO_MD5_H__


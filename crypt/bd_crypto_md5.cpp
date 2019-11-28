

#include <fcntl.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#ifdef WIN32
#include <io.h>
#endif

#include <stdio.h>
#include "bd_crypto_md5.h"
#include "md5.h"

#define MAX_BUFFER_SIZE 4096

void bd_crypto_md5::encodeUC16(const char* in, unsigned long inLen, unsigned char out[16]) 
{
    md5_calc (out, (const unsigned char*)in, inLen);
}

void bd_crypto_md5::encodeUC32(const char* in, unsigned long inLen, unsigned char out[32])
{
    md5_calc (out, (const unsigned char*)in, inLen);
}

std::string bd_crypto_md5::encodeUC16(const char* in, unsigned long inLen)
{
    unsigned char tmp[32];
    memset(tmp, 0, sizeof(tmp));

    bd_crypto_md5::encodeUC16(in, inLen, tmp);

    char encode[64];
    memset(encode, 0, sizeof(encode));

    sprintf(encode,
            "%02x%02x%02x%02x"
            "%02x%02x%02x%02x"
            "%02x%02x%02x%02x"
            "%02x%02x%02x%02x",
            tmp[0],  tmp[1],  tmp[2],  tmp[3],
            tmp[4],  tmp[5],  tmp[6],  tmp[7],
            tmp[8],  tmp[9],  tmp[10], tmp[11],
            tmp[12], tmp[13], tmp[14], tmp[15]);

    return std::string(encode);
}

std::string bd_crypto_md5::encodeUC32(const char* in, unsigned long inLen)
{
    unsigned char tmp[32];
    memset(tmp, 0, sizeof(tmp));

    bd_crypto_md5::encodeUC32(in, inLen, tmp);

    char encode[64];
    memset(encode, 0, sizeof(encode));

    sprintf(encode,
    		"%02x%02x%02x%02x"
			"%02x%02x%02x%02x"
			"%02x%02x%02x%02x"
			"%02x%02x%02x%02x",
            tmp[0],  tmp[1],  tmp[2],  tmp[3],
            tmp[4],  tmp[5],  tmp[6],  tmp[7],
            tmp[8],  tmp[9],  tmp[10], tmp[11],
            tmp[12], tmp[13], tmp[14], tmp[15]);

    return std::string(encode);
}

void bd_crypto_md5::encodeUL4(const char* in, unsigned long inLen, unsigned long out[4]) 
{
    unsigned char tmp[16];
    md5_calc (tmp, (const unsigned char*)in, inLen);

    out[0] = tmp[3]  + (((unsigned long)tmp[2]) << 8)
             + (((unsigned long)tmp[1]) << 16)
             + (((unsigned long)tmp[0]) << 24);

    out[1] = tmp[7]  + (((unsigned long)tmp[6]) << 8)
             + (((unsigned long)tmp[5]) << 16)
             + (((unsigned long)tmp[4]) << 24);

    out[2] = tmp[11]  + (((unsigned long)tmp[10]) << 8)
             + (((unsigned long)tmp[9]) << 16)
             + (((unsigned long)tmp[8]) << 24);

    out[3] = tmp[15]  + (((unsigned long)tmp[14]) << 8)
             + (((unsigned long)tmp[13]) << 16)
             + (((unsigned long)tmp[12]) << 24);
}

bool bd_crypto_md5::encodeFileUC16( const std::string fileName, unsigned char out[16] )
{
#ifdef WIN32
    int fd = open(fileName.c_str(), O_BINARY|O_RDONLY);
#else
    int fd = open(fileName.c_str(), O_RDONLY);
#endif

    if(fd < 0)
    {
        return false;
    }

//    int fileLen = 0;
    unsigned char buff[MAX_BUFFER_SIZE];

    long readLen = 0;
    MD5_CTX context;
    MD5Init( &context );
    while((readLen = read(fd, buff, MAX_BUFFER_SIZE)) > 0)
    {
        MD5Update( &context, buff, readLen );
    }
    MD5Final( out, &context);
    if(readLen < 0)
    {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}


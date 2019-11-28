
#pragma once


#ifndef Base64_h__
#define Base64_h__

extern const char*  ch64;
extern void Base64Encode(unsigned char *src,int srclen,char* szOut);
extern void Base64Decode(char *src,unsigned char* lpOut);

extern char *base64_encode(const char *src,unsigned int nLength);
extern char *base64_decode(const char *src);

#endif // Base64_h__
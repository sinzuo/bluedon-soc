//#include "../common/stdafx.h"
#include <stdlib.h>
// #include <stdio.h>
#include <string.h>
// #include <malloc.h>

const char*  ch64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 

void Base64Encode(unsigned char *src,int srclen,char* szOut)
{
	int n,buflen,i,j;
	int pading=0;
	unsigned char *buf;
	static unsigned char *dst; 

	buf=src;
	buflen=n=srclen;
	if(n%3!=0)  /* pad with '=' by using a temp buffer */
	{
		pading=1;
		buflen=n+3-n%3;
		buf=(unsigned char*)malloc(buflen+1);
		memset(buf,0,buflen+1);
		memcpy(buf,src,n);
		for(i=0;i<3-n%3;i++)
			buf[n+i]='=';
	}
	dst=(unsigned char*)malloc(buflen*4/3+1);
	memset(dst,0,buflen*4/3+1);
	for(i=0,j=0;i<buflen;i+=3,j+=4)
	{
		dst[j]=(buf[i]&0xFC)>>2;
		dst[j+1]=((buf[i]&0x03)<<4) + ((buf[i+1]&0xF0)>>4);
		dst[j+2]=((buf[i+1]&0x0F)<<2) + ((buf[i+2]&0xC0)>>6);
		dst[j+3]=buf[i+2]&0x3F; 
	}
	for(i=0;i<buflen*4/3;i++) /* map 6 bit value to base64 ASCII character */
		dst[i]=ch64[dst[i]];
	if(pading)
		free(buf);
	strcpy(szOut,(char*)dst);
	free(dst);
}

void Base64Decode(char *src,unsigned char* lpOut)
{
	int n,i,j;
	unsigned char *p;
	static unsigned char *dst; 

	n=(int)strlen(src);
	for(i=0;i<n;i++) /* map base64 ASCII character to 6 bit value */
	{
		p=(unsigned char*)strchr(ch64,src[i]);
		if(!p)
			break;
		src[i]=(char)(p-(unsigned char*)ch64);
	}
	dst=(unsigned char*)malloc(n*3/4+1);
	memset(dst,0,n*3/4+1);
	for(i=0,j=0;i<n;i+=4,j+=3)
	{
		dst[j]=(src[i]<<2) + ((src[i+1]&0x30)>>4);
		dst[j+1]=((src[i+1]&0x0F)<<4) + ((src[i+2]&0x3C)>>2);
		dst[j+2]=((src[i+2]&0x03)<<6) + src[i+3];
	}
	memcpy(lpOut,dst,n*3/4+1);
	free(dst);
}

static const char table[] = {
	'A' , 'B' , 'C' , 'D' , 'E' , 'F' , 'G',
	'H' , 'I' , 'J' , 'K' , 'L' , 'M' , 'N',
	'O' , 'P' , 'Q' , 'R' , 'S' , 'T' , 'U',
	'V' , 'W' , 'X' , 'Y' , 'Z' , 'a' , 'b',
	'c' , 'd' , 'e' , 'f' , 'g' , 'h' , 'i',
	'j' , 'k' , 'l' , 'm' , 'n' , 'o' , 'p',
	'q' , 'r' , 's' , 't' , 'u' , 'v' , 'w',
	'x' , 'y' , 'z' , '0' , '1' , '2' , '3',
	'4' , '5' , '6' , '7' , '8' , '9' , '+',
	'/' , '='
};

//base64编码
char *base64_encode(const char *src,unsigned int nLength)
{
	int count;
	char *dst;
	long tmp, buf;
	char in[4];

	int i, j;

	count = nLength / 3 + (nLength % 3 ? 1 : 0);
	dst =(char *)malloc(count * 4 + 1);
	memset(dst, 0, count * 4 + 1);

	for(j = 0; j < count; j++) {
		memset(in, 0, sizeof(in));
		strncpy(in, src + j * 3, 3);

		buf = 0;
		for(i = 0; i < strlen(in); i++) {
			tmp = (long)in[i];
			tmp <<= (16 - i * 8);
			buf |= tmp;
		}

		for(i = 0; i < 4; i++) {
			if(strlen(in) + 1 > i) {
				tmp = buf >> (18 - 6 * i);
				tmp &= 0x3f;
				dst[j * 4 + i] = table[tmp];
			} else {
				dst[j * 4 + i] = '=';
			}
		}
	}

	return dst;
}

//base64解码
char *base64_decode(const char *src)
{
	int count, len;
	char *dst;

	long tmp, buf;
	int i, j, k;
	char in[5];
	len = strlen(src);
	count = len / 4;

	dst = (char *)malloc(count * 3 + 1);
	memset(dst, 0, count * 3 + 1);

	for(j = 0; j < count; j++) {
		memset(in, 0, sizeof(in));
		strncpy(in, src + j * 4, 4);

		buf = 0;
		for(i = 0; i < 4; i++) {
			tmp = (long)in[i];
			if(tmp == '=') {
				tmp = 0;
			} else {
				for(k = 0; ; k++) {
					if(table[k] == tmp)
						break;
				}
				tmp = k;
			}
			tmp <<= (18 - i * 6);
			buf |= tmp;
		}

		for(i = 0; i < 3; i++) {
			tmp = buf >> (16 - i * 8);
			tmp &= 0xff;
			dst[j * 3 + i] =(char)tmp;
		}
	}

	return dst;
}



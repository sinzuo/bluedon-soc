#include <stdio.h>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include "bd_https.h"
#include <openssl/err.h>
#include <list>
#include <map>
#include <fstream>
#include <vector>
using namespace netprotocol;
using namespace std;

int mydata_index=SSL_get_ex_new_index(1, (void*)"mydata index", NULL, NULL, NULL);
netprotocol::bd_https::bd_https():bd_netbase(),certmap(NULL)
{
	m_curl = curl_easy_init();   //added in 2016.12.19 by wb
}

netprotocol::bd_https::~bd_https()
{
/*	
	if(certmap!=NULL)
	{
		certmap->clear();
		delete(certmap);
		certmap=NULL;
	}
*/

	if(certmap!=NULL)
	{
		std::map<struct certsha,X509*>::iterator it;
		for (it = certmap->begin(); certmap->end() != it; ++it)
		{
			X509_free(it->second);
			it->second = NULL;
		}
		certmap->clear();
		delete(certmap);
		certmap=NULL;
	}
	
	curl_easy_cleanup(m_curl);   //added in 2016.12.19 by wb
}

unsigned char* unbase64(char *input, int length, int* outLen)
{
	EVP_ENCODE_CTX  ctx;
	int orgLen = (((length+2)/4)*3) + 1;
	unsigned char* orgBuf = new unsigned char[orgLen];
	int result, tmpLen;
	EVP_DecodeInit(&ctx);
	EVP_DecodeUpdate(&ctx, (unsigned char *)orgBuf, &result, (unsigned char *)input, length);
	EVP_DecodeFinal(&ctx, (unsigned char *)&orgBuf[result], &tmpLen);

	result += tmpLen;
	*outLen = result;
	return orgBuf;
}

void netprotocol::bd_https::customverify(const std::string& certfilename) throw(std::runtime_error)
{
    return ;  // 功能和应用场景不完善，暂时不用该函数功能，后续需要再完善优化

	if ( certfilename.substr(certfilename.length()-1, 1) == "/" )
	{
		throw std::runtime_error("cert file not exist");
	}

	ifstream in(certfilename.c_str());
	//in.open(certfilename.c_str());
	if(!in.is_open())
	{
		throw std::runtime_error("Couldnt open file");
	}
	char strstart[]="-----BEGIN CERTIFICATE-----";
	char strend[]="-----END CERTIFICATE-----";
	bool startread=false;
	string bufstring;
	while(in.peek() != EOF)
	{
		char linebuffer[1024];
		in.getline(linebuffer,1024);
		if(memcmp(linebuffer,strstart,sizeof(strstart)-1)==0)
		{
			//startpos=in.tellg();
			startread=true;
			bufstring.clear();
		}
		else if(memcmp(linebuffer,strend,sizeof(strend)-1)==0)
		{
			startread=false;
			int certlen=0;
			size_t strlen=bufstring.length();
			unsigned char certbuf[2048]={0};

			//certbuf = unbase64(certbuffer,length,&certlen);
			certlen=EVP_DecodeBlock((unsigned char*)certbuf,(const unsigned char*)bufstring.c_str(),strlen);

			//理论上，编码(encode)时，如果输入串不是3的倍数，会在后面补0,以保持3的倍数，
			//反映到encode后的串，就是后面对应补了'='， '='在正常base64编码中不会存在，
			//因此，base64解码时有能力去除尾部的'\0'(虽然上述有些函数没有这么干）。
			string::reverse_iterator iterator;
			for(iterator=bufstring.rbegin();iterator!=bufstring.rend(); ++iterator)
			{
				if(*iterator == '=')
				{
					certlen--;
				}
			}

			X509 * x = X509_new();
			x509_st * px = NULL;

			const unsigned char* pDataCertCopy = certbuf;//!!! 必须用备份出来的指针进行操作openSSL操作, 这个指针会被OpenSSL改掉

			px = d2i_X509(&x, &pDataCertCopy, certlen);

			unsigned char micromap[32]={0};

			SHA1(certbuf,certlen, micromap);
			size_t errlen = 1000;
			char errBuf[1001] = {0};
			//std::string errMsg = ERR_error_string(errlen, errBuf);
			//size_t errCode = ERR_get_error();
			if(px != NULL && x!=NULL)
			{
				struct certsha c(micromap);
				certmap=new std::map<struct certsha,X509*>;
				certmap->insert(std::pair<struct certsha,X509*>(c, x));
			}
		}
		else
		{
			if(startread)
			{

				bufstring+=linebuffer;
				if(!bufstring.empty() && *bufstring.rbegin() == '\r')
				{
					bufstring.erase( bufstring.length()-1, 1);
				}
			}
		}
	}
	in.close();

}

int verify_callback(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
  X509 *cert = X509_STORE_CTX_get_current_cert(x509_ctx);
  int depth = X509_STORE_CTX_get_error_depth(x509_ctx);
  //int err = X509_STORE_CTX_get_error(x509_ctx);

  //save the certificate by incrementing the reference count and keeping a pointer
  /*if (depth < MAX_CERTS && !certificate[depth]) {
      certificate[depth] = cert;
      certificate_error[depth] = err;
  }*/
  SSL *ssl=NULL;
  ssl = (SSL*)X509_STORE_CTX_get_ex_data(x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
  SSL_CTX* ctx=SSL_get_SSL_CTX(ssl);

  struct verifyctx *myctx=NULL;
  myctx= (struct verifyctx *)SSL_CTX_get_ex_data(ctx, mydata_index);
  if(myctx->is_verify_success)
  {
	  return 1;
  }
  else
  {
	  unsigned int temp=0;
	  X509_digest(cert, EVP_sha1(), cert->sha1_hash, &temp);
	  struct certsha csha(cert->sha1_hash);
	  std::map<struct certsha,X509*>::iterator it;
	  it=myctx->certmap->find(csha);
	  if(it!=myctx->certmap->end())
	  {
		  myctx->is_verify_success=true;
		  X509 * x=it->second;
		  X509_free(x);
		  it->second = NULL;
		  //delete(myctx->certmap);
		  //printf("free\n");
		  return 1;
	  }
	  else if(depth==0)
	  {
		  return 0;
	  }
  }
  return 1;
}

CURLcode sslctxfun(CURL *curl, SSL_CTX *sslctx, void *parm)
{
	if(parm!=NULL)
	{
		struct verifyctx* ctx=(struct verifyctx*)parm;

		//mydata_index = SSL_get_ex_new_index(1, (void*)"mydata index", NULL, NULL, NULL);
		SSL_CTX_set_ex_data(sslctx, mydata_index, ctx);
		SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, verify_callback);
	}
	else
	{
		SSL_CTX_set_verify(sslctx, SSL_VERIFY_NONE, NULL);
	}
	return CURLE_OK;
}

static size_t getcontent_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t len=size*nmemb;
	string *str=(string*)userp;
	str->append((char*)buffer,len);
	return len;
}

int bd_https::test_request_page(const std::string &url, int *costTime)
{
    CURLcode res; double dCostTime;
	std::string pagecontent;
	
	if(NULL == m_curl)  
    {  
        return CURLE_FAILED_INIT;  
    }  
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());	

	//返回http response内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,getcontent_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &pagecontent);

	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);	//忽略认证服务器的证书
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);	//如果服务器用了其他host name,忽略认证

	//调试信息
	curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
	
	/** 
    * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。 
    * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。 
    */  
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 10);
	
	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(m_curl);

    if(res == CURLE_OK)
    {
        curl_easy_getinfo(m_curl, CURLINFO_TOTAL_TIME, &dCostTime);
        dCostTime = dCostTime * 1000; //dCostTime由秒转为毫秒
    }
    else dCostTime = 0;

	curl_easy_reset(m_curl);	//删掉之前的参数

    *costTime = (int)dCostTime;
    //返回状态码
	return res;
}

std::string bd_https::get_request_page(const std::string &url, std::string headers)
{
	buildstate();	//重新设定登录账号密码
	CURLcode res;
	std::string pagecontent;
    struct curl_slist *clist = NULL;

	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());	

    //设置http头信息
    if(!headers.empty())
    {
        string::size_type pos;
        while((pos = headers.find("&")) != string::npos)
        {
            string header = headers.substr(0, pos);
            clist = curl_slist_append(clist, header.c_str());
            headers = headers.substr(pos + 1);
        }
        if(!headers.empty())
            clist = curl_slist_append(clist, headers.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, clist);
    }

	//返回http response内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,getcontent_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &pagecontent);

	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);	//忽略认证服务器的证书
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);	//如果服务器用了其他host name,忽略认证

    struct verifyctx ctx;
    if(this->certmap!=NULL)
    {

		ctx.certmap=this->certmap;
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, sslctxfun);  //自定义验证
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_DATA, &ctx);  //自定义验证
    }
	//调试信息
	curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
	

	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(m_curl);

	long retcode;
	curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE , &retcode); 

    curl_easy_reset(m_curl);	//删掉之前的参数

    curl_slist_free_all(clist); /* free the list again */

	return pagecontent;

}

std::string bd_https::post_request_page(const std::string &url, const std::string& post_str, std::string headers)throw(bd_net_exception)
{
	buildstate();	//重新设定登录账号密码
	CURLcode res;
	std::string pagecontent;
    struct curl_slist *clist = NULL;

	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	
	/* Now specify the POST data */ 
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, post_str.c_str());

    //设置http头信息
    if(!headers.empty())
    {
        string::size_type pos;
        while((pos = headers.find("&")) != string::npos)
        {
            string header = headers.substr(0, pos);
            clist = curl_slist_append(clist, header.c_str());
            headers = headers.substr(pos + 1);
        }
        if(!headers.empty())
            clist = curl_slist_append(clist, headers.c_str());
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, clist);
    }

	//返回http response内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,getcontent_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &pagecontent);

	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);	//忽略认证服务器的证书
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);	//如果服务器用了其他host name,忽略认证
    struct verifyctx ctx;
    if(this->certmap!=NULL)
	{
		ctx.certmap=this->certmap;
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, sslctxfun);  //自定义验证
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_DATA, &ctx);  //自定义验证
	}
	//调试信息
	curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
	

	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(m_curl);
    
	long retcode;
	curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE , &retcode); 

	curl_easy_reset(m_curl);	//删掉之前的参数
	
	if(res==CURLE_COULDNT_CONNECT)
	{
		throw bd_net_exception("could not connect",retcode);
	}
	else if(res == CURLE_SSL_CACERT)
	{
		throw bd_net_exception("problem with the CA cert",retcode);
	}
	else if(res == CURLE_REMOTE_ACCESS_DENIED)
	{
		throw bd_net_exception("REMOTE_ACCESS_DENIED",retcode);
	}
	else if(res == CURLE_REMOTE_FILE_NOT_FOUND)
	{
		throw bd_net_exception("REMOTE_FILE_NOT_FOUND",retcode);
	}

    curl_slist_free_all(clist); /* free the list again */

	return pagecontent;

}

static size_t filedownload_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	FILE *file = (FILE*)userp;

	fwrite(buffer,size,nmemb,file);
	//printf("%s\n",buffer);
	return size*nmemb;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t retcode = fread(ptr, 1, size*nmemb, (FILE*)stream);
	//fprintf(stderr, "*** We read %d bytes from file\n", retcode);
	return retcode;
}

void netprotocol::bd_https::post_request_downloadfile(
		const std::string& url,const std::string& post_str,
		const std::string& savefilename)throw(bd_net_exception)
{
	CURLcode res;
	buildstate();
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	/* Now specify the POST data */
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, post_str.c_str());

	FILE * file = fopen(savefilename.c_str(),"wb");
	if(!file) {
		throw std::runtime_error("Couldnt open file");	
	}
    //fseek(file,0,SEEK_SET);

	//返回http response内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,filedownload_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, file);

	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);	//忽略认证服务器的证书
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);	//如果服务器用了其他host name,忽略认证
	struct verifyctx ctx;
	if(this->certmap!=NULL)
	{

		ctx.certmap=this->certmap;
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, sslctxfun);  //自定义验证
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_DATA, &ctx);  //自定义验证
	}

	/* Perform the request, res will get the return code */
	curl_easy_perform(m_curl);
	fclose(file);
	double retcode;
	res = curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE , &retcode);
	curl_easy_reset(m_curl);	//删掉之前的参数

	if(res==CURLE_COULDNT_CONNECT)
	{
		throw bd_net_exception("could not connect",retcode);
	}
	else if(res == CURLE_REMOTE_ACCESS_DENIED)
	{
		throw bd_net_exception("REMOTE_ACCESS_DENIED",retcode);
	}
	else if(res == CURLE_REMOTE_FILE_NOT_FOUND)
	{
		throw bd_net_exception("REMOTE_FILE_NOT_FOUND",retcode);
	}

}

string netprotocol::bd_https::post_request_uploadfile(
		const string& url,map<string,string>& postdata,
		const string& filename,const string& file_field)
		throw(bd_net_exception,std::logic_error)
{
	CURLcode res;
	//std::string filebasename;
	std::string pagecontent;
	//int timeout = 120;

	buildstate();	//重新设定登录账号密码

	struct curl_httppost *post=NULL;
	struct curl_httppost *last=NULL;

	std::map<string,string>::iterator it;

	for(it=postdata.begin();it!=postdata.end(); ++it)
	{
		curl_formadd(&post, &last, CURLFORM_COPYNAME, it->first.c_str(),
				CURLFORM_COPYCONTENTS, it->second.c_str(),
				CURLFORM_END);
	}

	curl_formadd(&post, &last, CURLFORM_COPYNAME, file_field.c_str(),
		CURLFORM_FILE, filename.c_str(),
		CURLFORM_END);

	curl_easy_setopt(m_curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, post);
	//curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, timeout);	//貌似多线程不能使用

	//返回http response内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,getcontent_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &pagecontent);

	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0L);	//忽略认证服务器的证书
	curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0L);	//如果服务器用了其他host name,忽略认证
	struct verifyctx ctx;
	if(this->certmap!=NULL)
	{

		ctx.certmap=this->certmap;
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_FUNCTION, sslctxfun);  //自定义验证
		curl_easy_setopt(m_curl, CURLOPT_SSL_CTX_DATA, &ctx);  //自定义验证
	}

	res = curl_easy_perform(m_curl);

	if(CURLE_OK != res)
	{
		char errstr[100];
		sprintf(errstr,"can not upload the file.Errno: %d\n", res);
		throw bd_net_exception(errstr,res);
	}

	curl_easy_reset(m_curl);	//删掉之前的参数


	curl_formfree(post);

	return pagecontent;

}

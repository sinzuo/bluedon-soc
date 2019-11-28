
#ifndef __BD_HTTPS__
#define __BD_HTTPS__
#include "netbase/bd_netbase.h"
#include "exception/bd_net_exception.h"
#include "common/bd_basic.h"
#include <openssl/ssl.h>
#include <memory.h>
#include <map>

using namespace std;

struct certsha
{
	unsigned char sha1_hash[SHA_DIGEST_LENGTH];

	certsha(unsigned char* hash)
	{
		memmove(sha1_hash,hash,SHA_DIGEST_LENGTH);
	}

	bool operator <(const struct certsha &cert) const
	{
		return memcmp(sha1_hash,cert.sha1_hash,SHA_DIGEST_LENGTH)<0;
	}

	bool operator ==(const struct certsha &cert) const
	{
		return memcmp(sha1_hash,cert.sha1_hash,SHA_DIGEST_LENGTH);
	}
};

struct verifyctx
{
	map<struct certsha,X509*> *certmap;
	bool is_verify_success;

	verifyctx():is_verify_success(false)
	{

	}
};

namespace netprotocol
{

	class BD_EXPORT_CLASS bd_https:public bd_netbase
	{
	public:
		bd_https();
		~bd_https();
		
		/**  
		 *  http的get请求页面, 测试URL是否能被访问
         *  @param url 下载的url; costTime 需要获取的响应时间
         *  @return值: 请求的http状态值(0:正常,其他:异常)
		 */ 
        int test_request_page(const std::string& url, int *costTime);  //added in 2016.12.19 by wb
		/**  
		 *  http的get请求页面
		 *  @param url 下载的url
         *  @param headers 请求的附加http头; 格式"a:xxx&b:xxx&..."
		 *  @param pagecontent 请求的页面内容
		 *  @return?请求的http状态值
		 */ 
        std::string get_request_page(const std::string& url, std::string headers = "");
		/**  
		 *  http的post请求页面
		 *  @param url 下载的url
		 *  @param post_str post过去的字符串，如"name=daniel&project=curl"
         *  @param headers 请求的附加http头; 格式"a:xxx&b:xxx&..."
		 *  @param pagecontent 请求的页面内容
		 *  @return?请求的http状态值
		 */ 
        std::string post_request_page(const std::string& url, const std::string& post_str, std::string headers = "") throw(bd_net_exception);

		/**
		 *  http的post请求下载
		 *  @param url 下载的url
		 *  @param post_str post过去的字符串，如"name=daniel&project=curl"
		 *  @param savefilename 下载的文件保存目录
		 *  @return?请求的http状态值
		 */
		void post_request_downloadfile(
				const std::string& url,const std::string& post_str,
				const std::string& savefilename)throw(bd_net_exception);

		/**
		 *  http的post请求上传
		 *  @param url 下载的url
		 *  @param post_str post过去的字符串，如"name=daniel&project=curl"
		 *  @param filename 要上传的文件目录
		 *  @return?请求的http状态值
		 */
		std::string post_request_uploadfile(
				const std::string& url,map<string,string>& postdata,
				const string& filename,const string& file_field)
				throw(bd_net_exception,std::logic_error);
		/**
		 *  设置自定义证书验证
		 *  @param certfilename 证书存放的文件
		 */
		void customverify(const std::string& certfilename) throw(std::runtime_error);

	private:
		CURL *m_curl;  //modified in 2016.12.19 by wb
		//CURLcode res;
		//bool m_is_verifycert;
		std::map<struct certsha,X509*> *certmap;
	};


}

#endif

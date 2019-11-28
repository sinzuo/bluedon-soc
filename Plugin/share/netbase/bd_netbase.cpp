#include <curl/curl.h>
#include <sys/stat.h>
#include <memory.h>
#ifdef WIN32
	#include <direct.h>
#endif
#ifdef WIN32
#else
	#include <libgen.h>
	#include <errno.h>
#endif
#include "bd_netbase.h"
 
ACE_Thread_Mutex netprotocol::bd_netbase::s_mutex;

netprotocol::bd_netbase::bd_netbase() throw(runtime_error)
{
	memset(m_userpwd,0,sizeof(m_userpwd));

	{
		ACE_Guard<ACE_Thread_Mutex> gdSync(s_mutex);    //加锁
		curl_global_init(CURL_GLOBAL_ALL);
	}
	m_curl = curl_easy_init();

	if(!m_curl) 
	{
		throw runtime_error("curl init error");
	}

}

netprotocol::bd_netbase::~bd_netbase()
{
	curl_easy_cleanup(m_curl);
}

void netprotocol::bd_netbase::login(const std::string& path,const std::string& username,const std::string& password)
{

	m_serverurl=path;
	m_username=username;
	m_password=password;
	sprintf(m_userpwd,"%s:%s",m_username.c_str(),m_password.c_str());
}

void netprotocol::bd_netbase::buildstate()
{
	//login(m_serverurl,m_username,m_password);
	//curl_easy_setopt(m_curl, CURLOPT_URL, m_serverurl.c_str());
	if(m_userpwd)
	{
		curl_easy_setopt(m_curl, CURLOPT_USERPWD, m_userpwd); 
	}
		//调试信息
	curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

	//不加这个多线程有问题
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);

}

static size_t filedownload_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	FILE *file = (FILE*)userp;
	
	fwrite(buffer,size,nmemb,file);
	//printf("%s\n",buffer);
	return size*nmemb;
}

void netprotocol::bd_netbase::downloadfile(const std::string& url,const std::string& savefilename)throw(bd_net_exception)
{
	CURLcode res;
	buildstate();
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	
	FILE * file = fopen(savefilename.c_str(),"wb");
	if(!file) {
		throw std::runtime_error("Couldnt open file");
	}
    //fseek(file,0,SEEK_SET);
	
	//返回http response内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,filedownload_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, file);
	

	/* Perform the request, res will get the return code */ 
	curl_easy_perform(m_curl);
	fclose(file);
	int retcode;
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

void netprotocol::bd_netbase::getinfo(const std::string& url, struct netprotocol::fileinfo* info)
{
	CURLcode res;
	time_t filetime;
	double filesize;
	memset(info,0,sizeof(fileinfo));

	buildstate();	//重新设定登录账号密码

	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	/* No download if the file */ 
	curl_easy_setopt(m_curl, CURLOPT_NOBODY, 1L);
	/* Ask for filetime */ 
	curl_easy_setopt(m_curl, CURLOPT_FILETIME, 1L);
	/* No header output: TODO 14.1 http-style HEAD output for ftp */ 
	//curl_easy_setopt(m_curl, CURLOPT_HEADERFUNCTION, throw_away);
	curl_easy_setopt(m_curl, CURLOPT_HEADER, 0L);
	//调试信息
	//curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

	res = curl_easy_perform(m_curl);

	if(CURLE_OK != res) 
	{
		fprintf(stderr, "m_curl told us %d\n", res);
		return;
	}
	  /* http://m_curl.haxx.se/libcurl/c/curl_easy_getinfo.html */ 
	res = curl_easy_getinfo(m_curl, CURLINFO_FILETIME, &filetime);
	if((CURLE_OK == res) && filetime)
	{
		strcpy(info->filename,url.c_str());
		info->modifytime=filetime;
	}
	res = curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);
	if((CURLE_OK == res) && (filesize>0))
	{
		//printf("filesize %s: %0.0f bytes\n", filename, filesize);
		info->length=filesize;
	}
	curl_easy_reset(m_curl);	//删掉之前的参数
}


static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t retcode = fread(ptr, 1, size*nmemb, (FILE*)stream);
	//fprintf(stderr, "*** We read %d bytes from file\n", retcode);
	return retcode;
}

void netprotocol::bd_netbase::uploadfile(const std::string& url,const std::string& filename)throw(bd_net_exception,std::logic_error)
{
	CURLcode res;
	FILE *hd_src;
	struct stat file_info;
	curl_off_t fsize;
	std::string filebasename;
	
	buildstate();	//重新设定登录账号密码

	//BD_LOGINF("URL:[%s]",url.c_str());

#ifdef WIN32
	//char path_buffer[_MAX_PATH];  
	char drive[_MAX_DRIVE];  
	char dir[_MAX_DIR];  
	char fname[_MAX_FNAME];  
	char ext[_MAX_EXT];  
	_splitpath_s(filename.c_str(),drive,dir,fname,ext);
	filebasename.append(fname);
	filebasename.append(ext);
#else
	char filefullname[256];
	strcpy(filefullname,filename.c_str());
	char* fname=basename(filefullname);
	filebasename.append(fname);
#endif
	std::string fullurl=url+filebasename;

	/* get the file size of the local file */ 
	if(stat(filename.c_str(), &file_info)) 
	{
		char errstr[100];
		sprintf(errstr,"Couldnt open file:errno:%d\n", errno);
		throw std::logic_error(errstr);
	}
	fsize = (curl_off_t)file_info.st_size;
	
	/* get a FILE * of the same file */ 
	hd_src = fopen(filename.c_str(), "rb");
	//m_curl = curl_easy_init();
	/* we want to use our own read function */ 
	curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, read_callback);

	/* enable uploading */ 
	curl_easy_setopt(m_curl, CURLOPT_UPLOAD, 1L);

	/* specify target */ 
	curl_easy_setopt(m_curl,CURLOPT_URL, fullurl.c_str());

	/* now specify which file to upload */ 
	curl_easy_setopt(m_curl, CURLOPT_READDATA, hd_src);

	//得到文件的大小
	curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE,(curl_off_t)fsize);

	res = curl_easy_perform(m_curl);

	if(CURLE_OK != res) 
	{
		char errstr[100];
		sprintf(errstr,"can not upload the file.Errno: %d\n", res);
		//BD_LOGERR(errstr);

		throw bd_net_exception(errstr,res);
	}

	curl_easy_reset(m_curl);	//删掉之前的参数

	
	fclose(hd_src); /* close the local file */ 

}

static size_t getcontent_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t len=size*nmemb;
	string *str=(string*)userp;
	str->append((char*)buffer,len);
	return len;
}


void netprotocol::bd_netbase::command(const std::string& commandstr,std::string* result)throw(bd_net_exception)
{
	buildstate();	//重新设定登录账号密码
	if(m_serverurl.empty())
	{
		throw bd_net_exception("please login in the server");
	}

	struct curl_slist *headerlist=NULL;
	headerlist = curl_slist_append(headerlist, commandstr.c_str());

	curl_easy_setopt(m_curl, CURLOPT_URL, m_serverurl.c_str());

    curl_easy_setopt(m_curl, CURLOPT_POSTQUOTE, headerlist);

	//返回内容回调
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,getcontent_write_data );
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, result);

    curl_easy_perform(m_curl);
    int retcode;
    CURLcode res;
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


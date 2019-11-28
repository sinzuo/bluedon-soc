//sftp_libssh2.cpp文件清单
#include "bd_sftp_ssh.h"
    
//#ifdef HAVE_WINSOCK2_H
//#include <winsock2.h>
//#endif
//#ifdef HAVE_SYS_SOCKET_H
//#include <sys/socket.h>
//#endif
//#ifdef HAVE_NETINET_IN_H
//#include <netinet/in.h>
//#endif
//#ifdef HAVE_UNISTD_H
//#include <unistd.h>
//#endif
//#ifdef HAVE_ARPA_INET_H
//#include <arpa/inet.h>
//#endif
//#ifdef HAVE_SYS_TIME_H
//#include <sys/time.h>
//#endif
  
//#include <dirent.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <fcntl.h>
//#include <errno.h>
//#include <stdio.h>
//#include <ctype.h>
  

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#endif

#include <iomanip>
#include <sstream>
  
//#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "libeay32.lib")
//#pragma comment(lib, "libssh2.lib")
   


//bd_sftp_libssh2* bd_sftp_libssh2::m_instance = NULL;

netprotocol::bd_sftp_libssh2::bd_sftp_libssh2() :m_sockfd(0), m_isBreak(false)
{
    m_bkCall = NULL;
    m_session = NULL;
    m_sftp_session = NULL;
    strLastError = "Sftp starts!";
}

/* 初始化sftp前调用
 *返回值:如果非0表示初始化失败！
 */
int netprotocol::bd_sftp_libssh2::ssh_init()
{
    int rc = 0;
#ifdef WIN32
    WSADATA wsadata;
    rc = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if(rc != 0) return rc;
#endif

    rc = libssh2_init(0);
    return rc;
}

/*sftp结束的时候调用*/
void netprotocol::bd_sftp_libssh2::ssh_exit()
{
    libssh2_exit();
#ifdef WIN32
    WSACleanup();
#endif
}

std::string netprotocol::bd_sftp_libssh2::getLastError()
{
    return strLastError;
}

/*
源码参考地址: http://www.libssh2.org/examples/sftp_write.html
*/
int netprotocol::bd_sftp_libssh2::upload(std::string localpath, std::string remotepath)
{
    if(localpath.length() < 1 || remotepath.length() < 1) return -1;

    int nR = 0;
    FILE *localfp = NULL;

    if((localfp = fopen(localpath.c_str(), "rb")) == NULL)
    {
        std::ostringstream ostr;
        ostr << "Can't open local file " << localpath;
        strLastError = ostr.str();
        return -2;
    }

    //取待上传文件整个size.
    fseek(localfp, 0, SEEK_END);
    size_t filesize = ftell(localfp);//local file大小，在readFromDisk中被引用
    fseek(localfp, 0, SEEK_SET);//文件指针重置到文件头


    //向SFTP服务器发出新建文件请求
    LIBSSH2_SFTP_HANDLE *sftp_handle = libssh2_sftp_open(m_sftp_session, remotepath.c_str(), LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC,
                                                         LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH);

    if(!sftp_handle)
    {
        std::ostringstream ostr;
        ostr << "Unable to open file with SFTP. remotepath=[" << remotepath << "]";
        strLastError = ostr.str();

        fclose(localfp);
        return -3;
    }


    char mem[1024 * 16];
    size_t nread;
    char *ptr;
    size_t count = 0;
    int rc = 0;
    do {
        nread = fread(mem, 1, sizeof(mem), localfp);
        if (nread <= 0) //到达文件尾部
            break;

        ptr = mem;
        do {
            // 向服务器写数据，直到数据写完毕
            rc = libssh2_sftp_write(sftp_handle, ptr, nread);
            if(rc < 0) break;

            ptr += rc; count += nread;
            nread -= rc;

            //如果设置了回调，进行回调
            if(m_bkCall)
            {
                float p = count / (float)filesize;
                m_bkCall->OnProgress(p);
            }  //callback.end
        } while(nread);

        if(m_isBreak == true)
        {
            std::ostringstream ostr;
            ostr << "上传文件任务被取消!";
            strLastError = ostr.str();
            nR = -6;
            break;
        }
    } while(rc > 0);

    libssh2_sftp_close(sftp_handle);
    fclose(localfp);

    return nR;//返回0表示成功
}

/*
源码参考地址: http://www.oschina.net/code/snippet_12_10717
*/
int netprotocol::bd_sftp_libssh2::download(std::string localpath, std::string remotepath)
{
    int nR = 0;
    if(localpath.length() < 1 || remotepath.length() < 1) return -1;

    /* Request a file via SFTP */
    LIBSSH2_SFTP_HANDLE *sftp_handle = libssh2_sftp_open(m_sftp_session, remotepath.c_str(), LIBSSH2_FXF_READ, 0);

    if(!sftp_handle) {
        std::ostringstream ostr;
        ostr << "Open remotepath failed! [" << remotepath << "]"; //libssh2_sftp_last_error(m_sftp_session)
        strLastError = ostr.str();
        return -2;
    }

    FILE *stream = NULL;
    if((stream = fopen(localpath.c_str(), "wb")) != NULL)
    {
        do {
            char mem[1024];
            if(m_isBreak == true)
            {
                std::ostringstream ostr;
                ostr << "下载文件任务被取消!";
                strLastError = ostr.str();
                nR = -6;
                break;
            }
            /* loop until we fail */
            int rc = libssh2_sftp_read(sftp_handle, mem, sizeof(mem));

            if(rc > 0)
            {
                fwrite(mem, 1, rc, stream); //从内存到磁盘
            } else
                break;
        } while(1);

        fclose(stream);
    } else
    {
        std::ostringstream ostr;
        ostr << "New local file failed: " << localpath;
        strLastError = ostr.str();
    }

    libssh2_sftp_close(sftp_handle);

    return nR;
}

int netprotocol::bd_sftp_libssh2::getSftpFList(std::string remotepath, std::vector<std::string> &vecFilenames)
{
    LIBSSH2_SFTP_HANDLE *sftp_handle = libssh2_sftp_opendir(m_sftp_session, remotepath.c_str());

    if(!sftp_handle)
    {
        std::ostringstream ostr;
        ostr << "打开SFTP远程目录失败";
        strLastError = ostr.str();
        return -1;
    }
    //printf("libssh2_sftp_opendir() is done, now receive listing!\n");

    do {
        char mem[512];
        //char longentry[512];
        LIBSSH2_SFTP_ATTRIBUTES attrs;

        /* loop until we fail */
        int rc = libssh2_sftp_readdir(sftp_handle, mem, sizeof(mem), &attrs);
        if(rc > 0)
        {
            /* rc is the length of the file name in the mem buffer */
            if((attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) && LIBSSH2_SFTP_S_ISREG(attrs.permissions))
            {
                /* 只记录regular文件 */
                if(strncmp(mem, ".", 1) == 0)  continue; // 过滤 ".*" 隐藏文件
                vecFilenames.push_back(std::string(mem));
            }
        } else
            break;

    } while(1);

    libssh2_sftp_closedir(sftp_handle);
    return 0;
}

int netprotocol::bd_sftp_libssh2::rmSftpFileName(std::string filename)
{
    //printf("libssh2_sftp_opendir() is done, now receive listing!\n");

    int rc = libssh2_sftp_unlink(m_sftp_session, filename.c_str());

    return rc;

}

int netprotocol::bd_sftp_libssh2::getLocalFList(std::string localpath, std::vector<std::string> &vecFilenames)
{
    struct dirent *file;
    if(localpath.length() != localpath.rfind("/")+1)
        localpath += "/";   // 为目录最后一位补"/"
    DIR *dirPtr = opendir(localpath.c_str());
    if(dirPtr == NULL)
    {
        std::ostringstream ostr;
        ostr << "打开本地目录失败" << localpath;
        strLastError = ostr.str();
        return -1;
    }

     do {
        file = readdir(dirPtr); //读取待处理文件
        if(file == NULL) break;

        // 过滤 ".*" 隐藏文件
        if(strncmp(file->d_name, ".", 1) == 0)  continue;

        std::string strFilename = localpath + std::string(file->d_name);
        //文件状态获取失败，或者为不为普通文件 continue
        struct stat st;
        if(stat(strFilename.c_str(), &st) < 0 || !S_ISREG(st.st_mode)) continue;
        vecFilenames.push_back(std::string(file->d_name));
    } while(1);

    closedir(dirPtr);
    return 0;
}

bool netprotocol::bd_sftp_libssh2::sftpInit(std::string ip, unsigned short port,
                                            std::string username, std::string password)
{
    bool bR = false;
    if(ip.empty() || username.empty() || password.empty())
        return bR;

    unsigned long hostaddr;
    struct sockaddr_in sin;
    const char *fingerprint;

    //初始化libssh2
    int rc = ssh_init();
    if(rc != 0)
    {
        std::ostringstream ostr;
        ostr << "[" << __FILE__ << "][" << __LINE__ << "]ssh_init() failed!";
        strLastError = ostr.str();
        return bR;
    }

    //新建连接
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    hostaddr = inet_addr(ip.c_str());

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = hostaddr;
    rc = connect(m_sockfd, (struct sockaddr*)(&sin),sizeof(struct sockaddr_in));
    if(rc != 0)
    {
        std::ostringstream ostr;
        ostr << "[" << __FILE__ << "][" << __LINE__ << "]failed to connect " << ip << "!";
        strLastError = ostr.str();
        return bR;
    }

    //新建对话实例
    m_session = libssh2_session_init();
    if(!m_session)
    {
        std::ostringstream ostr;
        ostr << "[" << __FILE__ << "][" << __LINE__ << "]libssh2_session_init() failed!";
        strLastError = ostr.str();
        return bR;
    }

    //设置调用阻塞
    libssh2_session_set_blocking(m_session, 1);

    //进行握手
    rc = libssh2_session_handshake(m_session, m_sockfd);
    if(rc != 0) {
        std::ostringstream ostr;
        ostr << "[" << __FILE__ << "][" << __LINE__ << "]Failure establishing SSH session: " << rc;
        strLastError = ostr.str();
        return bR;
    }

    //检查主机指纹
    std::ostringstream ostr;
    fingerprint = libssh2_hostkey_hash(m_session, LIBSSH2_HOSTKEY_HASH_SHA1);
    ostr << "Fingerprint: ";
    for(int i = 0; i < 20; i++) {
        unsigned char c = fingerprint[i];
        int nT = c;
        ostr << std::hex << std::setw(2) << std::setfill('0') << nT;
    }
    strLastError = ostr.str();

    //通过密码验证登陆用户身份
    while ((rc = libssh2_userauth_password(m_session, username.c_str(), password.c_str())) ==
           LIBSSH2_ERROR_EAGAIN);
    if(rc) {
        std::ostringstream ostr;
        ostr << "[" << __FILE__ << "][" << __LINE__ << "]Authentication by password failed.";
        strLastError = ostr.str();
        return bR;
    }

    m_sftp_session = libssh2_sftp_init(m_session);

    if(!m_sftp_session) {
        std::ostringstream ostr;
        ostr << "[" << __FILE__ << "][" << __LINE__ << "]Unable to init SFTP session!";
        strLastError = ostr.str();
        return bR;
    }

    bR = true;
    return bR;
}

void netprotocol::bd_sftp_libssh2::sftpRelease()
    {
        if(m_sftp_session)
        {
           libssh2_sftp_shutdown(m_sftp_session);
           m_sftp_session = NULL;
        }

        if(m_session)
        {
            libssh2_session_disconnect(m_session, "Normal Shutdown, Thank you!");
            libssh2_session_free(m_session);
            m_session = NULL;
        }

        if(m_sockfd)
        {
#ifdef WIN32
            closesocket(m_sockfd);
#else
            close(m_sockfd);
#endif
            m_sockfd = 0;
        }

        ssh_exit();
    }

 

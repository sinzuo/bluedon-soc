#ifndef __BD_SFTP_SSH_H__
#define __BD_SFTP_SSH_H__

#include "libssh2.h"
#include "libssh2_sftp.h"

#include <string>
#include <vector>

  
/* 
功能：SFTP协议的文件传输功能 
参考代码链接：http://blog.csdn.net/lee353086/article/details/40350241
*/  
  
namespace netprotocol {  

    class SFTP_BKCall  
    {  
    public:  
        /* progress返回值范围[0.0,1.0] */  
        virtual void OnProgress(float progress) = 0;  
    };  

    class bd_sftp_libssh2
    {  
    public:  
        bd_sftp_libssh2();

        /***************入口参数使用说明***************
        *ip:   就填一个IP地址就好了，例如“127.0.0.1”。
        *port: 端口，SFTP服务器默认端口为22。
        *username:
        *password:
        *remotepath: 远程路径，例如“/root/a.txt”
        *localpath:  本地路径，例如“/temp/test.jpg”
        */
        //sftp初始化: 初始化失败,也需调用sftp_release()释放资源
        bool sftpInit(std::string ip, unsigned short port, std::string username, std::string password);
        void sftpRelease();

        int upload(std::string localpath, std::string remotepath);
        int download(std::string localpath, std::string remotepath);

        int getSftpFList(std::string remotepath, std::vector<std::string> &vecFilenames);
        int getLocalFList(std::string localpath, std::vector<std::string> &vecFilenames);

        //删除目录文件
        int rmSftpFileName(std::string filename);

        //设置回调函数
        void setBKCall(SFTP_BKCall *bkCall) { m_bkCall = bkCall; }

        //用于停止当前上传或下载线程  
        void stopTask() { m_isBreak = true; }

        //获取最近的错误信息
        std::string getLastError();

    protected:
        int ssh_init();
        void ssh_exit();

    private:
        //static bd_sftp_libssh2* m_instance;

        int              m_sockfd;
        LIBSSH2_SESSION *m_session;
        LIBSSH2_SFTP    *m_sftp_session;

        SFTP_BKCall     *m_bkCall;
        bool             m_isBreak; //带读写保护的bool值
        std::string      strLastError; //存放最近的错误信息
    };  
}  
 
 
#endif

#include "bd_ssh.h"
//#include "libssh2_config.h"

#ifdef WIN32

#else
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#endif


static int waitsocket(int socket_fd, LIBSSH2_SESSION *session);
char *exitsignal=(char *)"none";

netprotocol::bd_ssh::bd_ssh():sock(-1), session(0), channel(0)
{
}


netprotocol::bd_ssh::~bd_ssh()
{    
    printf("run\n");

	if (NULL != channel)
	{
        char *exitsignal=(char *)"none";
        int rc = 0;
        while( (rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN )
        {
            printf("rc==LIBSSH2_ERROR_EAGAIN\n");
            waitsocket(sock, session);
        }

        printf("rc:[%d]\n", rc);
        if( rc == 0 )
        {
            printf("rc==0\n");
            //exitcode = libssh2_channel_get_exit_status( channel );
            libssh2_channel_get_exit_status( channel );
            libssh2_channel_get_exit_signal(channel, &exitsignal, NULL, NULL, NULL, NULL, NULL);
        }

        //	if (exitsignal)
        //		printf("\nGot signal: %s\n", exitsignal);
        //	else
        //		printf("\nEXIT: %d bytecount: %d\n", exitcode, bytecount);

        printf("before libssh2_channel_free\n");
        libssh2_channel_free(channel);
        printf("after libssh2_channel_free\n");

        channel = NULL;
	}

    if (NULL != session)
    {
        libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
        libssh2_session_free(session);
    }

#ifdef WIN32
	closesocket(sock);
#else
	close(sock);
#endif
	//fprintf(stderr, "all done\n");

	libssh2_exit();
}

//登录
void netprotocol::bd_ssh::login(const std::string& path,const std::string& username,const std::string& password)
{
	if(path.find("ssh://") == 0)
	{
        m_serverhost=path.substr(6);
	}
	else
	{
//		m_serverurl=path;
        m_serverhost = path;
	}
    //判断url是否包含端口冒号
    if (m_serverhost.find(":") != string::npos){
        int port_index = m_serverhost.find(":");
        m_serverport = m_serverhost.substr(port_index+1);
        m_serverhost = m_serverhost.substr(0,port_index);
    }
    else{
        m_serverport = "22";
    }
	m_username=username;
	m_password=password;
	//sprintf(m_userpwd,"%s:%s",m_username.c_str(),m_password.c_str());
}

string netprotocol::bd_ssh::command(const string& command) throw(bd_net_exception)
{
    printf("run\n");

	string result;
    unsigned long hostaddr;
//    int sock;
    struct sockaddr_in sin;
    const char *fingerprint;
//    LIBSSH2_SESSION *session;
//    LIBSSH2_CHANNEL *channel;
    int rc;
    //int exitcode;

    int bytecount = 0;
    size_t len;
    LIBSSH2_KNOWNHOSTS *nh;
    int type;

#ifdef WIN32
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2,0), &wsadata);
#endif



    rc = libssh2_init (0);

    if (rc != 0) {
    	throw bd_net_exception("libssh2 initialization failed",rc);
    }

    hostaddr = inet_addr(m_serverhost.c_str());

    /* Ultra basic "connect to port 22 on localhost"
     * Your code is responsible for creating the socket establishing the
     * connection
     */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(std::atoi(m_serverport.c_str()));
    sin.sin_addr.s_addr = hostaddr;
    if (connect(sock, (struct sockaddr*)(&sin),sizeof(struct sockaddr_in)) != 0) {
    	throw bd_net_exception("failed to connect!",-1);
    }

    /* Create a session instance */
    session = libssh2_session_init();

    if (!session)
    {
        throw bd_net_exception("failed to session init!",-1);
    }

    /* tell libssh2 we want it all done non-blocking */
    libssh2_session_set_blocking(session, 0);


    /* ... start it up. This will trade welcome banners, exchange keys,
     * and setup crypto, compression, and MAC layers
     */
    while ((rc = libssh2_session_handshake(session, sock)) == LIBSSH2_ERROR_EAGAIN);
    if (rc) {
    	throw bd_net_exception("Failure establishing SSH session",-1);
    }

    nh = libssh2_knownhost_init(session);

    if(!nh) {
        /* eeek, do cleanup here */
        //return 2;
    	throw bd_net_exception("knownhost init error",-1);
    }

    /* read all hosts from here */
    libssh2_knownhost_readfile(nh, "known_hosts",LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    /* store all known hosts to here */
    //libssh2_knownhost_writefile(nh, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);
    libssh2_knownhost_writefile(nh, "known_hosts", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    fingerprint = libssh2_session_hostkey(session, &len, &type);

    if(fingerprint) {
        struct libssh2_knownhost *host;
#if LIBSSH2_VERSION_NUM >= 0x010206
        /* introduced in 1.2.6 */
        int check = libssh2_knownhost_checkp(nh, m_serverhost.c_str(), 22,

                                             fingerprint, len,
                                             LIBSSH2_KNOWNHOST_TYPE_PLAIN|
                                             LIBSSH2_KNOWNHOST_KEYENC_RAW,
                                             &host);
#else
        /* 1.2.5 or older */
        int check = libssh2_knownhost_check(nh, hostname,

                                            fingerprint, len,
                                            LIBSSH2_KNOWNHOST_TYPE_PLAIN|
                                            LIBSSH2_KNOWNHOST_KEYENC_RAW,
                                            &host);
#endif

        /*****
         * At this point, we could verify that 'check' tells us the key is
         * fine or bail out.
         *****/
    }
    else {
        //return 3;
    	throw bd_net_exception("fingerprint error",-1);
    }
    libssh2_knownhost_free(nh);


    if ( !m_password.empty() ) {
        /* We could authenticate via password */
        while ((rc = libssh2_userauth_password(session, m_username.c_str(), m_password.c_str())) ==
               LIBSSH2_ERROR_EAGAIN);        
        if (rc) {
        	throw bd_net_exception("Authentication by password failed.",-1);
        }
    }
    else {
        /* Or by public key */
        while ((rc = libssh2_userauth_publickey_fromfile(session, m_username.c_str(),
                                                         "/home/user/"
                                                         ".ssh/id_rsa.pub",
                                                         "/home/user/"
                                                         ".ssh/id_rsa",
                                                         m_password.c_str())) ==
               LIBSSH2_ERROR_EAGAIN);
        if (rc) {
            //fprintf(stderr, "\tAuthentication by public key failed\n");
            //goto shutdown;
        	throw bd_net_exception("Authentication by public key failed",-1);
        }
    }

#if 0
    libssh2_trace(session, ~0 );

#endif

    /* Exec non-blocking on the remove host */
    while( (channel = libssh2_channel_open_session(session)) == NULL &&

           libssh2_session_last_error(session,NULL,NULL,0) == LIBSSH2_ERROR_EAGAIN )
    {
        waitsocket(sock, session);
    }
    if( channel == NULL )
    {
        fprintf(stderr,"Error\n");
        printf("channel is NULL\n");
        throw bd_net_exception("channel is NULL.",-1);
        //exit( 1 );
    }
    while( (rc = libssh2_channel_exec(channel, command.c_str())) == LIBSSH2_ERROR_EAGAIN )
    {
        waitsocket(sock, session);
    }
    if( rc != 0 )
    {
        fprintf(stderr,"Error\n");
        printf("rc is NULL\n");
        throw bd_net_exception("rc is NULL.",-1);
        //exit( 1 );
    }
    for( ;; )
    {
        /* loop until we block */
        int rc;
        do
        {
            char buffer[0x4000];
            rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );

            if( rc > 0 )
            {
                int i;
                bytecount += rc;
                //fprintf(stderr, "We read:\n");
                for( i=0; i < rc; ++i )
                	result.append(1,buffer[i]);
                    //fputc( buffer[i], stderr);
                //fprintf(stderr, "\n");
            }
            else {
                //if( rc != LIBSSH2_ERROR_EAGAIN )
                    /* no need to output this for the EAGAIN case */
                    //fprintf(stderr, "libssh2_channel_read returned %d\n", rc);
            }
        }
        while( rc > 0 );

        /* this is due to blocking that would occur otherwise so we loop on
           this condition */
        if( rc == LIBSSH2_ERROR_EAGAIN )
        {
            waitsocket(sock, session);
        }
        else
            break;
    }
    //exitcode = 127;
    return result;
}


static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
    struct timeval timeout;
    int rc;
    fd_set fd;
    fd_set *writefd = NULL;
    fd_set *readfd = NULL;
    int dir;

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    FD_ZERO(&fd);

    FD_SET(socket_fd, &fd);

    // now make sure we wait in the correct direction
    dir = libssh2_session_block_directions(session);


    if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        readfd = &fd;

    if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        writefd = &fd;

    rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

    return rc;
}

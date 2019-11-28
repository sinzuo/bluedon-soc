#include "probuf/socmsg.pb.h"
#include "zmq.h"
#include <ctime>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "../config/BDOptions.h"
#include  "unistd.h"

//#include "Poco/DeflatingStream.h"
//#include "Poco/InflatingStream.h"

using namespace std;
/*using Poco::DeflatingOutputStream;
using Poco::DeflatingStreamBuf;
using Poco::InflatingInputStream;
using Poco::InflatingStreamBuf;
using namespace bd_soc_protobuf;

*/
int main(int argc, char *argv[]) {

	void *m_zmq_context = zmq_ctx_new();               
	//void *m_zmq_client  = zmq_socket(m_zmq_context, ZMQ_REQ);

	if( zmq_ctx_set (m_zmq_context, ZMQ_IO_THREADS, 5) != 0)
	{
		return -1;
	}

	/*收发测试*/
    void *m_zmq_server  = zmq_socket(m_zmq_context,ZMQ_XREP);//ZMQ_DEALER
    Message * Ptr_recv  = new Message();
    bool bLocalListen = true; //监听
    string strServerBindAddr = "tcp://*:5577";

	/**/
	int affinity = 50000;
	zmq_setsockopt (m_zmq_server, ZMQ_RCVHWM, &affinity, sizeof (affinity));
	affinity = 65535;
	zmq_setsockopt (m_zmq_server, ZMQ_RCVBUF, &affinity, sizeof (affinity));
			
			
    if( zmq_bind(m_zmq_server, strServerBindAddr.c_str()) != 0){
        cout<<"zmq_bind  Failed!" << endl;
        bLocalListen = false;
    }
	long  count = 0;
    while(bLocalListen)
    {
        char szMsg[10240] = {0};
        //printf("waitting...\n");
        errno = 0;
        int zmq_size = zmq_recv(m_zmq_server, szMsg, sizeof(szMsg), 0);
        //cout << "recv size :"<<zmq_size<< endl;
        if(zmq_size < 0)
        {
            printf("error = %s\n", zmq_strerror(errno));
            usleep(10000);
            continue;
        }
		/*

        string strSzMsg(szMsg, zmq_size);
        istringstream istr(strSzMsg);
        if(!Ptr_recv->ParseFromIstream(&istr))
        {
            cout << "protocalbuffer parseFromIstream failed..." << endl;
        }

        string strszMsg = Ptr_recv->content();
*/
		if(zmq_size != 5)
		{
			count++;
			cout << "size=" <<zmq_size << "count= " << count << endl;
		}			

       // sleep(2);
     }//*/
	    
    return 0;	
}

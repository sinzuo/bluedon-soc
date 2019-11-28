#include "probuf/socmsg.pb.h"
#include "zmq.h"
#include <ctime>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include "../config/BDOptions.h"
#include "Poco/DeflatingStream.h"
#include "Poco/InflatingStream.h"
#include  <unistd.h>


using namespace std;
using Poco::DeflatingOutputStream;
using Poco::DeflatingStreamBuf;
using Poco::InflatingInputStream;
using Poco::InflatingStreamBuf;
using namespace bd_soc_protobuf;


int main(int argc, char *argv[]) {

    if (argc < 4) {
        cout << "ERROR:输入参数格式错误..." << endl;
        cout << "举例：" << endl;
        cout << "    启动插件:  ./agent_client tcp://172.16.12.188:5559 0 \"插件标识|策略\" 加密模式(可选) 压缩模式(可选)" << endl;
        cout << "    停止插件:  ./agent_client tcp://172.16.12.188:5559 1 \"插件标识\" 加密模式(可选) 压缩模式(可选)" << endl;
        cout << "    启动传感器:  ./agent_client tcp://172.16.12.188:5559 3 \"all\" 加密模式(可选) 压缩模式(可选)" << endl;
        cout << "    停止传感器:  ./agent_client tcp://172.16.12.188:5559 4 \"all\" 加密模式(可选) 压缩模式(可选)" << endl;
        cout << "    下发策略:  ./agent_client tcp://172.16.12.188:5559 5 \"插件标识|策略\" 加密模式(可选) 压缩模式(可选)" << endl;
        return -1;
    }

    void *m_zmq_context = zmq_ctx_new();
    void *m_zmq_client  = zmq_socket(m_zmq_context, ZMQ_REQ);


    CBDOptions CBDtemp;
    CBDtemp.setCipher();

    /**/
    int nEnptype = 4000, nComtype = 5000;

    if (argc >= 6) {
        nEnptype = atoi(argv[4]);
        nComtype = atoi(argv[5]);

    } else if (argc == 5) {
        nEnptype = atoi(argv[4]);
    }

    Message_enptypeNo tmpenptype =  Message_enptypeNo(nEnptype);
    Message_comtypeNo tmpcomtype =  Message_comtypeNo(nComtype);

    zmq_connect(m_zmq_client, argv[1]);

    Message * PB_Ptr  = new Message();

    if (strcmp(argv[2], "0") == 0) {    // start plugin
        PB_Ptr->set_service_no(Message_ServiceNo_START_PLUGIN);

    } else if (strcmp(argv[2], "1") == 0) { // stop  plugin
        PB_Ptr->set_service_no(Message_ServiceNo_STOP_PLUGIN);

    } else if (strcmp(argv[2], "3") == 0) { // stop  agent, add in 2017.3.1
        PB_Ptr->set_service_no(Message_ServiceNo_START_AGENT);

    } else if (strcmp(argv[2], "4") == 0) { // stop  agent, add in 2017.3.1
        PB_Ptr->set_service_no(Message_ServiceNo_STOP_AGENT);

    } else if (strcmp(argv[2], "5") == 0) {
        PB_Ptr->set_service_no(Message_ServiceNo_START_TASK);

    } else {
        cout << "ERROR:非法参数..." << endl;
    }

    time_t t = time(NULL);
    PB_Ptr->set_time((unsigned long long)t);
    PB_Ptr->set_serial_no(999999);

    string str = argv[3];
    string strOut = str;

    /*加解密测试*/
    if (tmpenptype == Message_enptypeNo_NO_ENPTYPE) { // 不加密
        str = str;

    } else if (4000 < tmpenptype && tmpenptype < 4006) { // 加密
        str = CBDtemp.EncodePw(str, tmpenptype);

    } else {
        cout << "加密模式参数错误" << endl;
    }

    /*压缩测试*/
    if (tmpcomtype == Message_comtypeNo_NO_COMPRESS) {
        str = str;

    } else if (tmpcomtype == Message_comtypeNo_COMPRESS_ZLIB) { // 压缩
        cout << "ZLIB压缩" << endl;
        ostringstream conostr;  //zip data
        DeflatingOutputStream deflater(conostr, DeflatingStreamBuf::STREAM_ZLIB);
        deflater << str;
        deflater.close();  //To ensure buffers get flushed before connected stream closed
        str = conostr.str();

    } else if (tmpcomtype == Message_comtypeNo_COMPRESS_GZIP) { // 压缩
        cout << "GZIP压缩" << endl;
        ostringstream conostr;  //zip data
        DeflatingOutputStream deflater(conostr, DeflatingStreamBuf::STREAM_GZIP);
        deflater << str;
        deflater.close();
        str = conostr.str();

    } else {
        cout << "压缩模式参数错误" << endl;
    }

    PB_Ptr->set_enptype(tmpenptype);
    PB_Ptr->set_comtype(tmpcomtype);

    PB_Ptr->set_content(str);
    /**/
    ostringstream ostr;
    PB_Ptr->SerializeToOstream(&ostr);
    int rec = zmq_send(m_zmq_client, ostr.str().c_str(), ostr.str().length(), 1);
    cout << "send to server with content =>[" << strOut << "]" << endl;
    cout << "ZMQ发送结果: [" << rec << "]" << endl;

    char szMsg[1024] = {0};
    int zmq_client = zmq_recv(m_zmq_client, szMsg, sizeof(szMsg), 0);
    cout << "recv size :" << zmq_client << endl;


    /*收发测试*/
    void *m_zmq_server  = zmq_socket(m_zmq_context, ZMQ_REP); //ZMQ_DEALER
    Message * Ptr_recv  = new Message();
    bool bLocalListen = true; //监听
    string strServerBindAddr = "tcp://*:5559";
    string bindPort = string(argv[1]).substr(string(argv[1]).rfind(":") + 1);

    if (atoi(bindPort.c_str()) != 0) {
        strServerBindAddr = string("tcp://*:") + bindPort;
    }


    if (zmq_bind(m_zmq_server, strServerBindAddr.c_str()) != 0) {
        cout << "zmq_bind  Failed!" << endl;
        bLocalListen = false;
    }

    while (bLocalListen) {
        char szMsg[1024] = {0};
        printf("waitting...\n");
        errno = 0;

        int zmq_size = zmq_recv(m_zmq_server, szMsg, sizeof(szMsg), 0);
        cout << "recv size :" << zmq_size << endl;

        if (zmq_size < 0) {
            printf("error = %s\n", zmq_strerror(errno));
            sleep(2);
            continue;
        }

        //rec = zmq_send(m_zmq_server, szMsg, sizeof(szMsg), 1);
        string strSzMsg(szMsg, zmq_size);
        istringstream istr(strSzMsg);

        if (!Ptr_recv->ParseFromIstream(&istr)) {
            cout << "protocalbuffer parseFromIstream failed..." << endl;
        }

        int tmpEnType = Ptr_recv->enptype();
        int tmpCoType = Ptr_recv->comtype();
        int serial  = Ptr_recv->serial_no();
        string strszMsg = Ptr_recv->content();

        /*解压*/
        if (tmpcomtype == Message_comtypeNo_COMPRESS_ZLIB || tmpcomtype == Message_comtypeNo_COMPRESS_GZIP) { // 压缩
            istringstream istr(strszMsg);
            InflatingStreamBuf::StreamType type = InflatingStreamBuf::STREAM_ZLIB;

            if (tmpcomtype != Message_comtypeNo_COMPRESS_ZLIB) {
                type = InflatingStreamBuf::STREAM_GZIP;
            }

            InflatingInputStream infalter(istr, type);

            string strTmp;

            while (infalter.good()) {        // loop while extraction from file is possible
                char c = infalter.get();    // get character from file
                strTmp.append(1, c);

            }

            if (strTmp.size() > 0) {
                strTmp.erase(strTmp.end() - 1);
            }

            strszMsg = strTmp;

        }

        /*解密*/
        strszMsg = CBDtemp.DecodePw(strszMsg, tmpenptype);

        printf("received serial : %d\n", serial);
        printf("received tmpEnType : %d\n", tmpEnType);
        printf("received tmpCoType : %d\n", tmpCoType);
        printf("received message : %s\n", strszMsg.c_str());
        sleep(2);
    }//*/

    return 0;
}



#ifndef __BD_ICMP_CHECK_H__
#define	__BD_ICMP_CHECK_H__


#include <string>
#include<string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
using namespace std;
/* IP首部长度 */
#define IP_HEADER_LEN sizeof(struct ip)
/* icmp报文长度 */
#define ICMP_PACKET_LEN sizeof(struct icmp)
/* IP + ICMP长度 */
#define IP_ICMP_PACKET_LEN 1024

typedef struct _bd_icmp_data{

    string ip;
    int  type;
    int  code;
    float time;
    _bd_icmp_data()
    {
        type =-1;
        code = -1;
        time = 0;
    }

}bd_icmp_data;

class bd_icmp_check {
public:
    bd_icmp_check();
    ~bd_icmp_check();
    
    int  get_icmp_message(string dst_ip, int icmp_type, int icmp_sequ, bd_icmp_data *icmp_data_ptr);
    
private:
    /* 校验和 */
    unsigned short check_sum(unsigned short *addr, int len);
    /* 填充icmp报文 */
    struct icmp *fill_icmp_packet(int icmp_type, int icmp_sequ);
    /* 计算发送时间与接收时间的毫秒差 */
    float time_interval(struct timeval *recv_time, struct timeval *send_time);
    /* 接收icmp报文 */
    int icmp_recv(bd_icmp_data *icmp_data_pt);
    /* 发送icmp请求 */
    int icmp_request(const char *dst_ip, int icmp_type, int icmp_sequ);
    
private:
    int sockfd;
    int send_pid;
};

#endif	/* __BD_ICMP_CHECK__ */


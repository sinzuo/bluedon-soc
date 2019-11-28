#include "bd_icmp_check.h"

bd_icmp_check::bd_icmp_check()
{
    send_pid = 0;
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);

}
bd_icmp_check::~bd_icmp_check()
{
    close(sockfd);
}

/* 校验和 */
unsigned short bd_icmp_check::check_sum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    if(nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

/* 填充icmp报文 */
struct icmp *bd_icmp_check::fill_icmp_packet(int icmp_type, int icmp_sequ)
{
    struct icmp *icmp_packet;

    icmp_packet = (struct icmp *)malloc(ICMP_PACKET_LEN);
    icmp_packet->icmp_type = icmp_type;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_id = htons(getpid());
    send_pid = getpid();
    printf("send_icmp_packet->icmp_id :%d\n", getpid());
    icmp_packet->icmp_seq = htons(icmp_sequ);
    /* 发送时间 */
    gettimeofday((struct timeval *)icmp_packet->icmp_data, NULL);
    /* 校验和 */
    icmp_packet->icmp_cksum = check_sum((unsigned short *)icmp_packet, ICMP_PACKET_LEN);

    return icmp_packet;
}
/* 计算发送时间与接收时间的毫秒差 */
float bd_icmp_check::time_interval(struct timeval *recv_time, struct timeval *send_time)
{
    float msec = 0;

    /* 如果接收的时间微妙小于发送的微妙 */
    if (recv_time->tv_usec < send_time->tv_usec)
    {
        recv_time->tv_sec -= 1;
        recv_time->tv_usec += 1000000;
    }
    msec = (recv_time->tv_sec - send_time->tv_sec) * 1000.0 + (recv_time->tv_usec - send_time->tv_usec) / 1000.0;

    return msec;
}

int  bd_icmp_check::icmp_recv(bd_icmp_data *icmp_data_pt)
{
    struct ip *ip_header;
    struct icmp *icmp_packet;
    char buf[IP_ICMP_PACKET_LEN] = {0};
    struct timeval recv_timeval, *send_timeval;
    int ret_len;

    short  i_code = 0;
    struct in_addr sourceIP, destIP;


    struct timeval tv_out;
    tv_out.tv_sec = 5;   //设置了recv 5秒超时
    tv_out.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv_out,sizeof(struct timeval));
    int flag = 1;
    //端口复用
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)
    {
        //perror("setsockopt");
        printf("setsockopt() error!\n");
        return -1;
        //exit(1);
    }

   // recv_timeval = malloc(sizeof(struct timeval));

    while (1)
    {

        ret_len = recv(sockfd, buf, IP_ICMP_PACKET_LEN, 0);
        if(ret_len == -1 && errno == EAGAIN )
        {
            printf("recv() timeout!\n");
            return 0;
        }
        else if (ret_len > 0)
        {
            // 接收时间
            gettimeofday(&recv_timeval, NULL);
            // 取出ip首部
            // 取出icmp报文
            ip_header = (struct ip *)buf;
            icmp_packet = (struct icmp *)(buf + IP_HEADER_LEN);
            printf("size = %d \n", ret_len);
            memcpy(&i_code, buf+20+8+20+4, sizeof(i_code));
            memcpy(&sourceIP, buf+20+8+12, sizeof(sourceIP));
            memcpy(&destIP, buf+20+8+16, sizeof(destIP));
            //取出发送时间

            if(icmp_packet->icmp_type == 0 && icmp_packet->icmp_code == 0)
            {
                if(ntohs(icmp_packet->icmp_id) != send_pid)
                    continue;
            }
            else
            {
                if(string(inet_ntoa(destIP)) != icmp_data_pt->ip)
                    continue;
            }

            send_timeval = (struct timeval *)icmp_packet->icmp_data;
            printf("===============================\n");
            printf("from ip:%s\n", inet_ntoa(ip_header->ip_src));
            printf("icmp_type:%d\n", icmp_packet->icmp_type);
            printf("icmp_code:%d\n", icmp_packet->icmp_code);
            printf("icmp_id  :%d\n",ntohs(icmp_packet->icmp_id));
            printf("i_code  :%d\n",ntohs(i_code));
            printf("sourceIP  :%s\n",inet_ntoa(sourceIP));
            printf("destIP  :%s\n",inet_ntoa(destIP));
            printf("time interval:%.3fms\n", time_interval(&recv_timeval, send_timeval));

            icmp_data_pt->code =  icmp_packet->icmp_code;
            icmp_data_pt->type =  icmp_packet->icmp_type;
            if(time_interval(&recv_timeval, send_timeval) > 0)
            {
                icmp_data_pt->time =  time_interval(&recv_timeval, send_timeval);
            }
            break;
        }
        else
        {
            printf("recv() error!\n");
            return -1;
        }
    }
    //free(recv_timeval);
    return 0;
}

/* 发送icmp请求 参数：ip,icmp类型，序列号 */

int bd_icmp_check::icmp_request(const char *dst_ip, int icmp_type, int icmp_sequ)
{
    struct sockaddr_in dst_addr;
    struct icmp *icmp_packet;
    int ret_len;
    char buf[ICMP_PACKET_LEN];


    if(dst_ip == NULL)
        return -1;
    /* 请求的地址 */
    memset(&dst_addr, 0, sizeof(struct sockaddr_in));
    dst_addr.sin_family = AF_INET;
    //dst_addr.sin_port = htons(12345);
    dst_addr.sin_addr.s_addr = inet_addr(dst_ip);


    //int  ttl = 2;
    //setsockopt(sockfd, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(ttl));

    /* icmp包 */
    icmp_packet = fill_icmp_packet(icmp_type, icmp_sequ);
    memcpy(buf, icmp_packet, ICMP_PACKET_LEN);

    /* 发送请求 */
    ret_len = sendto(sockfd, buf, ICMP_PACKET_LEN, 0, (struct sockaddr *)&dst_addr, sizeof(struct sockaddr_in));
    if (ret_len > 0)
    {
        printf("sendto() ip:%s ok!!!\n", dst_ip);
        free(icmp_packet);
        return 0;
    }
    /**/
    free(icmp_packet);
    return -1;
}

int bd_icmp_check::get_icmp_message(string dst_ip, int icmp_type, int icmp_sequ, bd_icmp_data *icmp_data_ptr)
{
    /**/

    int res = icmp_request(dst_ip.c_str(), icmp_type, icmp_sequ);
    if(res != 0) return -1;
    icmp_data_ptr->ip = dst_ip;
    res = icmp_recv(icmp_data_ptr);
    if(res != 0) return -2;
    return 0;

}

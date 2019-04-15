#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_PORT 5006 // 服务器进行对外的端口
#define SERVER_IP "127.0.0.1" // 服务器所在机器的IP

#define print_error(str) \
do{\
    fprintf(stderr, "File %s, Line %d, Function %s error\n",__FILE__, __LINE__, __func__);\
    perror(str);\
    exit(-1);\
}while(0);

/* tcp通信时发送的数据(send函数使用)，这里以一个学生信息为例，用于第5步 */
typedef struct tcpdata{
    unsigned int stu_num; // 学号,无符号整型
    char stu_name[30]; // 学生姓名
}student;

int cfd = -1; // 服务器端用于和指定客户端通信的指定fd,用于第4步

/* 线程处理函数，用于从读取客户端发送过来的消息,用于第6步 */
void *pth_func(void *path_arg)
{
    int ret = 0;
    student stu_data = {0};

    while(1){
        bzero(&stu_data, sizeof(stu_data)); // 清空结构体中的数据
        ret = recv(cfd, &stu_data, sizeof(stu_data), 0); 
        if(ret == -1) print_error("recv fail");
    
        printf("student number = %d\n", ntohs(stu_data.stu_num)); // 打印学号，记得把数据从网络端序转为主机断端序
        printf("student name = %s\n", stu_data.stu_name); // 1个字符存储的无需进行端序转换
    } 
}

/* 信号处理函数，用于接收外部信号停止服务器线程 */
void signal_func(int signo)
{
    if(signo == SIGINT){
        // close(cfd); // 断开服务器端和客户端通信的fd
        shutdown(cfd, SHUT_RDWR); // 读写都断开
        exit(0); // 退出服务器进程
    }
}

int main(int argc, char const *argv[])
{
    int ret = -1;

    /* 第5步：注册信号处理函数，用于关闭socket和退出线程 */
    signal(SIGINT, signal_func);

    /* 第1步：创建使用TCP协议通信的套接字文件，客户端的套接字直接用于和服务器端通信*/
    int skfd = -1;
    skfd = socket(AF_INET, SOCK_STREAM, 0);
    if(skfd == -1) print_error("socket fail");
    
    /* 第2步：调用connect主动向服务器发起三次握手，进行连接 */

    /* 第4步：注册线程函数，用于接收客户端的消息 */
    pthread_t recv_thread_id; // 接收客户端消息的线程id
    pthread_create(&recv_thread_id, NULL, pth_func, NULL); // 注册消息接收线程

    /* 第3步：服务器调用write(send)给指定客户端发送数据 */
    student stu_data = {0};
    unsigned int tmp_num; // 定义临时变量用于字节序转换
    while(1){
        // 获取学生学号,但是需要从主机端序转换为网络端序
        printf("Please input student number:\n"); 
        scanf("%d", &tmp_num);
        stu_data.stu_num = htonl(tmp_num);
        // 获取学生姓名，不需要进行端序转换,因为字符数组以一个字节为单位进行存储
        printf("Please input student name:\n"); // 获取学生姓名
        scanf("%s", stu_data.stu_name);
        // 根据cfd向指定的客户端发送数据
        ret = send(cfd, (void *)&stu_data, sizeof(stu_data), 0);
        if(ret == -1) print_error("send fail");
    }

    return 0;
}

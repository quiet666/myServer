#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PTHREAD_NUM 3

struct SockInfo{ 
    //含有子线程所需信息的结构体
    struct sockaddr_in addrc;  //存储socket网络信息
    int cfd;                    // socket文件标识
    pthread_t pid;              // 线程标识符
};

struct SockInfo sockInfoList[PTHREAD_NUM]; //总数组

void *pthread_work(void *arg); //线程工作内容


int main(){
    // 1. 创建套接字
    int fd;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd==-1){
        perror("服务器：创建套接字失败");
        exit(0);
    }

    // 2. 绑定端口与IP
    struct sockaddr_in addrs;
    addrs.sin_family = AF_INET;
    addrs.sin_port = htons(11111);
    addrs.sin_addr.s_addr = INADDR_ANY; // 这个宏的值为0，即“0.0.0.0”，自动读取网卡IP

    int bind_status = bind(fd, (struct sockaddr *)&addrs, sizeof(addrs));
    if(bind_status == -1){
        perror("bind() error");
        exit(0);
    }

    // 3. 监听
    int listen_status = listen(fd, 128); // 第二个参数代表一次最多接收多少个客户端连接
    if(-1 == listen_status){
        perror("listen() error");
        exit(0);
    }


    // 4. 接收客户端连接请求,并启动多线程

    // 4.1 初始化
    for(int i=0; i< PTHREAD_NUM; i++){
        bzero(&sockInfoList[i], sizeof(sockInfoList[i]));
        sockInfoList[i].cfd = -1;
        sockInfoList[i].pid = -1;
    }

    while(1){
        // 4.2 找到空闲的SockInfo结构体
        struct SockInfo *pInfo=NULL;  // 指向空闲的SockInfo结构体
        for (int i=0;i<PTHREAD_NUM;i++){
            if(sockInfoList[i].cfd == -1){
                pInfo = &sockInfoList[i];
                break;
            }
        }
        if(pInfo == NULL){
            sleep(1);
            continue;
        }

        // 4.3 调用accept
        int addr_len=sizeof(pInfo->addrc);
        int cfd = accept(fd, (struct sockaddr*)&(pInfo->addrc), &addr_len );
        pInfo->cfd = cfd;

        // 4.4 启动多线程

        pthread_create(&pInfo->pid, NULL, pthread_work, pInfo);
        pthread_detach(pInfo->pid);
    }

    // 4.5 释放资源
    close(fd); //关闭监听socket
    return 0;

}

void* pthread_work(void *arg){
    /*
        1. 需要addrc结构体
        2. 需要cfd
        3. 需要保存线程标识符
    */
   struct SockInfo *pInfo = (struct SockInfo*)arg;

    // 打印客户端ip
    char ip[32]={0};  
    printf("客户端的IP为:%s, \n地址为: %d\n",
        inet_ntop(AF_INET, &(pInfo->addrc).sin_addr.s_addr,ip ,sizeof(ip)),
        ntohs(pInfo->addrc.sin_port));

    // 5. 和客户端通信
    int n=0;
    char buff[1024];
    while(1){
        // 接收信息
        memset(buff, 0, sizeof(buff));
        int len = recv(pInfo->cfd, buff, sizeof(buff), 0);
        if(len > 0){
            printf("客户端say:%s",buff);
            // sprintf(buff, "小C,你过来%d下\n",n++);
            send(pInfo->cfd, buff, strlen(buff)+1,0);
            
        }else if(len == 0){
            perror("服务器：客户端断开连接");
            break;
        }
        else{  
            perror("服务器:recv数据失败");
            break;
        }
    }

    // 6. 解绑，关闭socket
    pInfo->pid = -1; // 不要忘记重置
    close(pInfo->cfd);
    return NULL;

}

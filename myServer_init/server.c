#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(){
    // 1. 创建套接字
    int sfd;
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd==-1){
        perror("服务器：创建套接字失败");
        exit(0);
    }

    // 2. 绑定端口与IP
    struct sockaddr_in addrs;
    addrs.sin_family = AF_INET;
    addrs.sin_port = htons(11111);
    addrs.sin_addr.s_addr = INADDR_ANY; // 这个宏的值为0，即“0.0.0.0”，自动读取网卡IP

    int bind_status = bind(sfd, (struct sockaddr *)&addrs, sizeof(addrs));
    if(bind_status == -1){
        perror("bind() error");
        exit(0);
    }

    // 3. 监听
    int listen_status = listen(sfd, 128); // 第二个参数代表一次最多接收多少个客户端连接
    if(-1 == listen_status){
        perror("listen() error");
        exit(0);
    }


    // 4. 阻塞，直到接收客户端连接请求
    struct sockaddr_in addrc;
    unsigned int addrc_len = sizeof(addrc);
    int cfd = accept(sfd, (struct sockaddr *)&addrc, &addrc_len );
    if (-1 == cfd){
        perror("accpet has error");
        exit(0);
    }
    //打印客户端ip
    char ip[32]={0};  
    printf("客户端的IP为:%s, \n地址为: %d\n",
        inet_ntop(AF_INET, &addrc.sin_addr.s_addr,ip ,sizeof(ip)),
        ntohs(addrc.sin_port));


    // 5. 和客户端通信

    int n=0;
    while(1){

        // 接收信息
        char buff[1024];
        memset(buff, 0, sizeof(buff));
        int len = recv(cfd, buff, sizeof(buff), 0);
        if(len > 0){
            printf("客户端say:%s",buff);
            // sprintf(buff, "小C,你过来%d下\n",n++);
            send(cfd, buff, strlen(buff)+1,0);
            
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
    close(sfd);
    close(cfd);

}

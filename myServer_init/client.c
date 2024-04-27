#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(){
    // 1. 创建套接字
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd==-1){
        perror("创建套接字失败");
        exit(0);
    }

    // 2. 绑定端口与IP
    
    struct sockaddr_in addrs;
    addrs.sin_family = AF_INET;
    addrs.sin_port = htons(11111);
    // addrs.sin_addr.s_addr = INADDR_ANY; // 这个宏的值为0，即“0.0.0.0”，自动读取网卡IP
    inet_pton(AF_INET,"192.168.31.154",&addrs.sin_addr.s_addr);

    int ret = connect(sfd,(struct sockaddr *)&addrs, sizeof(addrs));
    if(ret==-1){
	perror("connect 失败");
    }
    


    // 与服务器通信

    int n=0;

    printf("开始通信\n");
    while(1){
        // 发送消息
        char buff[1024];
        sprintf(buff, "向服务器大人第%d次问好...\n",n++);
        int ref = write(sfd, buff, strlen(buff)+1);
        if (ref == -1){
            perror("客户端无法发送消息给服务器");
        }


        // 接收服务器消息
        memset(buff, 0, sizeof(buff));
        int len = read(sfd, buff, sizeof(buff));

        if(len > 0){
            printf(" %s", buff);
            memset(buff, 0, sizeof(buff));

        }else if(len == 0){
            perror("服务器断开连接...");
            break;

        }
        else{
            perror("接受数据失败");
            break;
        }
        sleep(1);
    }

    // 6. 解绑，关闭socket
    close(sfd);
    return 0;

}

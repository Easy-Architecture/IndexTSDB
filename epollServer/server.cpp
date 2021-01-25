//
// Created by maomao on 2021/1/25.
//

#include "../gloabl.h"
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}
int main(int argc,char *argv[])
{
    int sockfd,cfd;
    char client_IP[1024],buf[1024];
    struct sockaddr_in sevraddr,cliaddr; //定义服务器地址结构 和客户端地址结构
    socklen_t clit_addr_len;   //客户端地址结构大小
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0))< 0)
    {
       sys_err("socket error!");
    }
    sevraddr.sin_family = AF_INET;
    sevraddr.sin_port = htons(9999);
    sevraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd,(struct sockaddr *)&sevraddr,sizeof (sevraddr))<0){
        sys_err("bind error!");
    }

    if (listen(sockfd,128)<0){
        sys_err("listen error!");
    }
    clit_addr_len = sizeof (cliaddr);
    if ((cfd=accept(sockfd,(struct sockaddr *)&cliaddr,&clit_addr_len))<0)
    {
        sys_err("accept error!");
    }
    printf("client ip:%s port:%d\n",inet_ntop(AF_INET,&cliaddr.sin_addr.s_addr,client_IP,sizeof (client_IP)),
           ntohs(cliaddr.sin_port));//根据accept传出的参数,获取客户端ip和port
    while (1){
        int ret= read(cfd,buf,sizeof (buf));//读取客户端数据
        write(STDOUT_FILENO,buf,ret);//写到屏幕查看

        for (int i = 0; i < ret; i++)
        {
            if (buf[i] == 'q'){
                printf("system exit!");
                exit(0);
            }
            buf[i] = toupper(buf[i]);

        }


            write(cfd,buf,ret); //将大写,写回客户端
    }
    close(sockfd);
    close(cfd);
    return 0;
}

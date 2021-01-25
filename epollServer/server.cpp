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
    int n,num = 0;
    char client_IP[1024],buf[1024];

    struct sockaddr_in sevraddr,cliaddr; //定义服务器地址结构 和客户端地址结构
    struct epoll_event tep,ep[OPEN_MAX]; //tep:epoll_ctl参数 ep[]:epoll_wait参数
    socklen_t  cilen;   //客户端地址结构大小


    if ((sockfd = socket(AF_INET,SOCK_STREAM,0))< 0)
    {
       sys_err("socket error!");
    }
    int opt = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof (opt));//端口复用
    bzero(&sevraddr,sizeof (sevraddr));
    sevraddr.sin_family = AF_INET;
    sevraddr.sin_port = htons(SERV_PORT);
    sevraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd,(struct sockaddr *)&sevraddr,sizeof (sevraddr))<0){
        sys_err("bind error!");
    }

    if (listen(sockfd,128)<0){
        sys_err("listen error!");
    }
    int efd = epoll_create(OPEN_MAX);//创建epoll模型,efd指向红黑树根节点
    if (efd == -1)
        sys_err("epoll_create error!");

    tep.events = EPOLLIN;
    tep.data.fd = sockfd; //指定lfd的监听事件为读

    int res = epoll_ctl(efd,EPOLL_CTL_ADD,sockfd,&tep);
    if(res = -1)
        sys_err("epoll_ctl error");

    /*
     * 作用和while(true)一样但是底层实现不同
     * 编译前              编译后
      while (1)；   mov eax,1
                    test eax,eax
                    je foo+23h
                    jmp foo+18h

    for (；；)；     jmp foo+23h 　　
    对比之下，for (；；)指令少，不占用寄存器，而且没有判断跳转，比while (1)好。

也就是说两者在在宏观上完全一样的逻辑，但是底层完全不一样，for相对于来说更加简洁明了。
     * */
    for (; ;) {
        //epoll为server阻塞监听事件,ep为struct epoll_event类数组,OPEN_MAX为数组容量,-1表示永久阻塞
        ssize_t nready = epoll_wait(efd,ep,OPEN_MAX,-1);
        if (nready == -1)
            sys_err("epoll_wait error");

        for (int i = 0; i < nready; i++) {
            if (!(ep[i].events & EPOLLIN)) //如果不是"读"事件,继续循环
                continue;

            if (ep[i].data.fd == sockfd){ //判断满足事件的fd是不是sockfd
                 cilen=sizeof (cliaddr);
                int connfd = accept(sockfd,(struct sockaddr*)&cliaddr,&cilen);
                printf("received from %s at PORT %d\n",inet_ntop(AF_INET,&cliaddr.sin_addr,client_IP,sizeof (client_IP)),
                       ntohs(cliaddr.sin_port));
                printf("cfd %d ---client %d\n",connfd,++num);

                tep.events = EPOLLIN;
                tep.data.fd = connfd;
                res = epoll_ctl(efd,EPOLL_CTL_ADD,connfd,&tep);//加入红黑树
                if (res == -1)
                    sys_err("epoll_ctl error!");
            }else{
                sockfd =ep[i].data.fd;
                n =read(sockfd,buf,MAXLINE);
                if (n == 0){//读到0,说明客户端关闭连接
                    res = epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);//将该文件描述符从红黑树摘下来
                    if (res == -1)
                        sys_err("epoll_ctl error");
                    close(sockfd); //关闭与该客户端的连接
                    printf("client[%d] closed connection\n",sockfd);
                }else if (n <0){//出错
                    perror("read n<0 error:");
                    res = epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,NULL);
                    close(sockfd);
                }else{
                    for (int i = 0; i < n; i++) {
                        buf[i] = toupper(buf[i]);

                        write(STDOUT_FILENO,buf,n);
                        write(sockfd,buf,n);
                    }
                }
            }
        }
        if (close(sockfd)==-1)
            sys_err("close sockfd error");

        if (close(efd) == -1);
            sys_err("close sockfd error");
        return 0;
    }


}

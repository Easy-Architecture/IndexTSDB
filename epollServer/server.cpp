//
// Created by maomao on 2021/1/25.
//

#include "../gloabl.h"
void sys_err(const char *str)
{
    perror(str);
    exit(1);
}
int Accept(int fd,struct sockaddr *sa,socklen_t *salenptr){
    int n;
    again:
    if ((n=accept(fd,sa,salenptr))<0){
        if ((errno == ECONNABORTED) || (errno == EINTR))
            goto again;
        else
            sys_err("accept error");
    }

    return n;
}
int Close(int fd){
    int n;
    if ((n=close(fd))==-1)
        sys_err("close error");
    return n;
}
ssize_t Read(int fd,void *ptr,size_t nbytes)
{
    ssize_t n;
    again:
    if ((n=read(fd,ptr,nbytes)) == -1){
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}
ssize_t Write(int fd,const void *ptr,size_t nbytes)
{
    ssize_t  n;
    again:
    if ((n=write(fd,ptr,nbytes)) == -1)
    {
        if (errno == EINTR)
            goto again;
        else
            return -1;
    }
    return n;
}
int main(int argc,char *argv[]) {
    int i, listenfd, sockfd, connfd, efd, len, res, flag;
    int n, num = 0;
    char client_IP[INET_ADDRSTRLEN], buf[1024];
    ssize_t nready;
    socklen_t clilen;   //客户端地址结构大小
    struct sockaddr_in sevraddr, cliaddr; //定义服务器地址结构 和客户端地址结构
    struct epoll_event event, ep[OPEN_MAX]; //tep:epoll_ctl参数 ep[]:epoll_wait参数


    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        sys_err("socket error!");
    }
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//端口复用
    bzero(&sevraddr, sizeof(sevraddr));
    sevraddr.sin_family = AF_INET;
    sevraddr.sin_port = htons(SERV_PORT);
    sevraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr *) &sevraddr, sizeof(sevraddr)) < 0) {
        sys_err("bind error!");
    }

    if (listen(listenfd, 20) < 0) {
        sys_err("listen error!");
    }
    efd = epoll_create(OPEN_MAX);//创建epoll模型,efd指向红黑树根节点
    if (efd == -1)
        sys_err("epoll_create error!");

    //event.events = EPOLLET; //默认LT水平触发
    event.events = EPOLLIN | EPOLLET; //ET边沿触发
    event.data.fd = listenfd; //指定lfd的监听事件为读
    printf("Accepting connections ... \n");
    clilen = sizeof(cliaddr);
    connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, client_IP, sizeof(client_IP)),
           ntohs(cliaddr.sin_port));
    printf("cfd %d ---client %d\n", connfd, ++num);
    //fcntl是用来修改已经打开文件的属性的函数
    flag = fcntl(connfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connfd, F_SETFL, flag);

    event.data.fd = connfd;
    res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);
    if (res == -1)
        sys_err("epoll_ctl error");


    for (;;) {
        printf("epoll_wait begin\n");
        res = epoll_wait(efd, ep, 10, -1); //最多10个,阻塞监听
        printf("epoll_wait end res %d\n", res);

        if (ep[0].data.fd == connfd) {
            while ((len = read(connfd, buf, MAXLINE / 2)) > 0) //非阻塞读,轮询
                write(STDOUT_FILENO, buf, len);
        }
    }
    return 0;
}

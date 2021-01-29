//
// Created by maomao on 2021/1/29.
//
#include "../gloabl.h"



//事件驱动结构体
typedef struct epollReactor{
    int fd;                                             //要监听的文件描述符
    int events;                                         //对应的监听事件
    void (*call_back)(int fd,int events,void *arg);     //回调函数
    void *arg;                                          //泛型参数
    char buf[1024];                                     //缓冲区
    int buflen;                                         //缓冲区长度
    int epfd;                                           //红黑树根
    int status;                                         //是否在监听:1->在红黑树上(监听),0->不在(不监听)
    long last_active;                                   //记录每次加入红黑树 g_efd的时间值
}erStruct;

//全局epoll树的根
int gepfd = 0;
int i;
erStruct epollevents[EVENT_SIZE+1];
void readData(int fd,int events,void *arg);

//添加事件
void eventadd(int fd,int events,void(*call_back)(int ,int ,void *),void *arg,erStruct *er)
{
    er->fd =fd;                           //要监听的文件描述符
    er->events = events;                  //对应的监听事件
    er->call_back=call_back;
    struct epoll_event epv;
    epv.events = events;
    epv.data.ptr = er;                    //核心思想 将自定义结构体 封装到epoll_event结构体 自定义结构体内封装回调函数 在需要时可以调出使用
    epoll_ctl(gepfd,EPOLL_CTL_ADD,fd,&epv);//添加
}
//修改事件
void eventset(int fd,int events,void (*call_back)(int,int,void *),void *arg,erStruct *er)
{
    er->fd = fd;
    er->events = events;
    er->call_back = call_back;

    struct epoll_event epv;
    epv.events = events;
    epv.data.ptr = er;
    epoll_ctl(gepfd,EPOLL_CTL_MOD,fd,&epv);//修改

}
//删除事件
void eventdel(erStruct *ev,int fd,int events)
{
    printf("begin call %s\n",__FUNCTION__ );
    ev->fd = 0;
    ev->events = 0;
    ev->call_back = NULL;
    memset(ev->buf,0x00,sizeof (ev->buf));
    ev->buflen=0;
    struct epoll_event epv;
    epv.data.ptr = NULL;
    epv.events = events;
    epoll_ctl(gepfd,EPOLL_CTL_DEL,fd,&epv);//从树上移出
}
//发送数据
void senddata(int fd,int events,void *arg)
{
    printf("begin call %s\n",__FUNCTION__ );
    erStruct *ev =(epollReactor *)arg;
    write(fd,ev->buf,ev->buflen);
    eventset(fd,EPOLLIN,readData,arg,ev);

}
//读数据
void readData(int fd,int events,void *arg)
{
    printf("begin call %s\n",__FUNCTION__ );
    erStruct *ev = (epollReactor *)arg;
    ev->buflen = read(fd,ev->buf,sizeof (ev->buf)); //通过read函数获取 字符长度赋值给buflen
    if (ev->buflen>0) //读到数据
    {
        eventset(fd,EPOLLOUT,senddata,arg,ev);
    }
    else if (ev -> buflen ==0) //对方关闭连接
    {
        close(fd);
        eventdel(ev,fd,EPOLLIN);
    }
}
//新连接处理
void initAccept(int fd,int events,void *arg)
{
    printf("begin call %s,gepfd=%d\n",__FUNCTION__ ,gepfd);//__FUNCTION__ 函数名
    int i;
    struct sockaddr_in addr;
    socklen_t  len = sizeof (addr);
    int cfd = accept(fd,(struct sockaddr*)&addr,&len);//是否会阻塞?
    //查找myevents数组中可用的位置
    for (i = 0; i < EVENT_SIZE;i ++) {
        if (epollevents[i].fd ==0)//0标准输入 1标准输出 2标准错误
        {
            break;
        }

    }
    //设置读事件
    eventadd(cfd,EPOLLIN,readData,&epollevents[i],&epollevents[i]);
}
int startServer(int port)
{
    //创建socket
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    //端口复用
    int opt = 1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof (opt));
    //绑定
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//0.0.0.0 本机所有ip 不论那块网卡
    bind(lfd,(struct sockaddr*)&servaddr,sizeof (servaddr));
    //监听
    listen(lfd,128);

    //创建epoll树根节点
    gepfd = epoll_create(1024);
    printf("gepfd == %d\n",gepfd);
    struct epoll_event events[1024];
    //添加最初始事件,将帧听的描述符添加到epoll树上
    eventadd(lfd,EPOLLIN,initAccept,&epollevents[EVENT_SIZE],&epollevents[EVENT_SIZE]);

    for(;;)
    {
        //监听红黑树g_efd,将满足的事件的文件描述符加至events数组中,1秒没有事件满足,返回0
        int nready = epoll_wait(gepfd,events,1024,-1);
        if (nready < 0) //调用epoll_wait失败
        {
            perror("epoll_wait error");
            if (errno == EINTR)
            {
                continue;
            }
            break;
        } else if (nready >0) //调用epoll_wait成功,返回有事件发生的文件描述符的个数
        {
            int i = 0;
            for (i = 0; i <nready ; i++) {
                erStruct *ers =(epollReactor *)events[i].data.ptr;//取ptr指向结构体地址
                printf("fd=%d\n",ers->fd);
                if (ers->events & events[i].events)
                {
                    ers->call_back(ers->fd,ers->events,ers);//调用事件按对应的回调
                }
            }
        }
    }
    //关闭监听文件描述符
    close(lfd);
    return 0;
}



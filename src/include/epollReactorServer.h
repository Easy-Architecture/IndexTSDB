//
// Created by maomao on 2021/1/29.
//

#ifndef INDEXTSDB_EPOLLSERVER_H
#define INDEXTSDB_EPOLLSERVER_H
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

int startServer(int port);

#endif //INDEXTSDB_EPOLLSERVER_H

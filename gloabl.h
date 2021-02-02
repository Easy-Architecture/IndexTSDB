//
// Created by maomao on 2021/1/6.
//

#ifndef ADVANCED_EXERCISES_GLOABL_H
#define ADVANCED_EXERCISES_GLOABL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <uv.h>
using namespace std;
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <vector>
#include <string.h>
#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <sstream>
#include <condition_variable>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <pthread.h>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "threadPool/threadPool.h"
#include "epollServer/epollReactorServer.h"
#define TESTPATH "/tmp/test.txt"
#endif //ADVANCED_EXERCISES_GLOABL_H

#define MAXLINE 8192
#define SERV_PORT 9999
#define OPEN_MAX 5000
#define MAX_EVENTS 1024  //监听数上限
#define BUFLEN 4096
#define EVENT_SIZE 1024
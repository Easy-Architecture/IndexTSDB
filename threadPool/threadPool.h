//
// Created by maomao on 2021/1/31.
//
#include <pthread.h>
#ifndef INDEXTSDB_THREADPOOL_H
#define INDEXTSDB_THREADPOOL_H
typedef struct{
    void *(*function)(void *); //函数指针,回调函数
    void *arg; //上面函数的参数
}threadPool_task; //各子线程任务结构体

//描述线程池相关信息
struct threadPool{
    pthread_mutex_t lock; //用于锁住本结构体
    pthread_mutex_t thread_counter; //记录忙状态线程个数的锁 -- busy_thr_num
    pthread_cond_t queue_not_full; //当任务队列满时,添加任务的线程阻塞,等待此条件变量
    pthread_cond_t queue_not_empty; //任务队列里不为空时,通知等待任务的线程

    pthread_t *threads; //存放线程池中每个线程的tid.数组
    pthread_t manager_tid; //存管理线程tid
    threadPool_task *task_queue; //任务队列

    int min_thr_num; //线程池最小线程数
    int max_thr_num; //线程池最大线程数
    int live_thr_num; //当前存活线程个数
    int busy_thr_num; //忙状态线程个数
    int wait_exit_thr_num; //要销毁的线程个数

    int queue_front; //task_queue队头下标
    int queue_rear; //task_queue 队尾下标
    int queue_size; //task_queue队中实际任务数
    int queue_max_size; //task_queue队列可容纳任务数上限
    int shutdown; //标志位,线程池使用状态,true为关闭,false为不关闭
};
threadPool * threadPool_create(int min_thr_num,int max_thr_num,int queue_max_size);
int threadPool_add(threadPool *pool,void*(*function)(void *arg),void *arg);
#endif //INDEXTSDB_THREADPOOL_H
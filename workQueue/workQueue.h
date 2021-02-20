//
// Created by maomao on 2021/2/2.
//

#ifndef INDEXTSDB_WORKQUEUE_H
#define INDEXTSDB_WORKQUEUE_H
#include "../gloabl.h"
const unsigned int max_push_number = 400 * 1000; //队列的最大长度
const unsigned int empty_wait = 1000; //空闲等待的延时
const uint32_t max_mmap_size = 200 * 1000 * 1000;  //共享内存的最大长度

void *cache_thread(void *);
void *execute_sql_thread(void *);
class sql_cache{
public:
    sql_cache(){}
    ~sql_cache();

public:
    int init();
    int run();

public:
    uint8_t *mmap_buf;              //  共享内存数据
    uint8_t *pop_mem;               //  pop队列
    uint8_t *cache_mem;             //  cache队列
    uint32_t mmap_buf_offset;       //  当前读取共享内存的偏移
    uint32_t pop_mem_offset;        //  当前pop队列的读取偏移
    uint32_t pop_mem_length;        //  当前pop队列的长度
    uint32_t cache_mem_length;      //  当前cache队列的长度
    pthread_mutex_t pop_lock;       //  pop队列的读写锁
    pthread_mutex_t swap_lock;      //  交换pop队列与cache队列数据的锁
    pthread_cond_t pop_fill;        //  pop队列填充数据的信号
    pthread_cond_t cache_clear;     //  cahce队列为空的信号
};

#endif //INDEXTSDB_WORKQUEUE_H


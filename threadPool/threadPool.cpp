//
// Created by maomao on 2021/1/31.
//

#include "../gloabl.h"
#define DEFAULT_TIME 10 /*10s检测一次*/
#define MIN_WAIT_TASK_NUM 10 /*如果queue_size > MIN_WAIT_TASK_NUM添加新的线程到线程池*/
#define DEFAULT_THREAD_VARY 10 /*每次创建和销毁线程的个数*/
#define true 1
#define false 0



void *threadPool_thread(void *threadpool); //线程池中的工作线程
void *manager_thread(void *threadPool); //管理者线程
int is_thread_alive(pthread_t tid); //检测线程是否存活
int threadPool_free(threadPool *pool); //释放线程池

//线程池中的各个工作线程
void  *threadPool_thread(void *threadpool)
{
    threadPool *pool = (threadPool *) threadpool;
    threadPool_task task;

    for (;;) {
        //刚创建出线程,等待任务队列里有任务,否则阻塞等待任务队列有任务后再唤醒接收任务
        pthread_mutex_lock(&(pool->lock));
        //queue_size == 0 说明没有任务 阻塞在条件变量上,若有任务,跳过该while
        while ((pool->queue_size == 0)&&(!pool->shutdown)){
            printf("thread_0x%x is waiting\n",(unsigned int)pthread_self());
            pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));
            //清楚指定数目的空闲线程,如果要结束线程的个数大于0,结束线程
            if (pool->wait_exit_thr_num > 0){
                pool->wait_exit_thr_num--;
                //如果线程池里线程个数大于最小值时可以结束当前线程
                if (pool->live_thr_num>pool->min_thr_num){
                    printf("thread 0x%x is exiting\n",(unsigned int )pthread_self());
                    pool->live_thr_num --;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(NULL);
                }
            }
        }
        //如果指定了 true,要关闭线程池里每个线程,自行退出处理
        if (pool->shutdown){
            pthread_mutex_unlock(&(pool->lock));
            printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
            pthread_exit(NULL); //线程自行结束
        }
        //从任务队列里获取任务,是一个出队操作
        task.function = pool->task_queue[pool->queue_front].function;
        task.arg = pool->task_queue[pool->queue_front].arg;

        pool->queue_front = (pool->queue_front + 1)%pool->queue_max_size;//出队 模拟环形队列
        pool->queue_size --;
        //通知可以有新的任务添加进来
        pthread_cond_broadcast(&(pool->queue_not_full));
        //任务取出后,立即将线程池锁释放
        pthread_mutex_unlock(&(pool->lock));
        //执行任务
        printf("thread 0x%x start working\n",(unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->thread_counter)); //忙状态线程数变量锁
        pool->busy_thr_num++;                        //忙状态线程数+1
        pthread_mutex_unlock(&(pool->thread_counter));
        (*(task.function))(task.arg);               //执行回调函数任务

        //任务结束处理
        printf("thread 0x%x end working\n",(unsigned int)pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num--; //处理掉一个任务,忙状态线程数-1
        pthread_mutex_unlock(&(pool->thread_counter));
    }
    pthread_exit(NULL);

}
threadPool * threadPool_create(int min_thr_num,int max_thr_num,int queue_max_size)
{
    int i;
    threadPool *pool = NULL;
    do{
        if ((pool=(threadPool *)malloc(sizeof(threadPool))) == NULL)
        {
            printf("malloc threadpool fail");
            break;
        }
        pool->max_thr_num = max_thr_num;
        pool->min_thr_num = min_thr_num;
        pool->busy_thr_num = 0;
        pool->live_thr_num = min_thr_num; //初始存活线程数 初值=最小线程数
        pool->queue_size = 0; //有0个产品
        pool->queue_max_size = queue_max_size;
        pool->queue_front = 0;
        pool->queue_rear = 0;
        pool->shutdown = false; //不关闭线程
        //根据最大线程上限数,给工作线程数组开辟空间并清零
        pool->threads = (pthread_t *) malloc(sizeof (pthread_t)* max_thr_num);
        if (pool->threads == NULL){
            printf("malloc threads fail");
            break;
        }
        memset(pool->threads,0,sizeof (pthread_t)*max_thr_num);
        //队列开辟内存
        pool->task_queue = (threadPool_task *) malloc(sizeof (threadPool_task)*queue_max_size);
        if (pool->task_queue == NULL)
        {
            printf("malloc task_queue fail");
            break;
        }
        //初始化互斥锁 条件变量 参数attr为空(NULL)，则使用默认的互斥锁属性，默认属性为快速互斥锁 。互斥锁的属性在创建锁的时候指定，在LinuxThreads实现中仅有一个锁类型属性，不同的锁类型在试图对一个已经被锁定的互斥锁加锁时表现不同。
        if (pthread_mutex_init(&(pool->lock),NULL)!=0
        || pthread_mutex_init(&(pool->thread_counter),NULL)!=0
        || pthread_cond_init(&(pool->queue_not_empty),NULL)!=0
        || pthread_cond_init(&(pool->queue_not_full),NULL)!=0)
        {
            printf("init the lock or cond fail");
            break;
        }
        for (i = 0; i <min_thr_num ; i++) {
            pthread_create(&(pool->threads[i]),NULL,threadPool_thread,(void *)pool);//*pool 指向当前线程池
            printf("start thread 0x%x...\n",(unsigned int)pool ->threads[i]);
        }
        pthread_create(&(pool->manager_tid),NULL,manager_thread,(void *)pool); //启动管理者线程
        return pool;
    } while (0);
    threadPool_free(pool);  //前面代码出错,释放pool存储空间
}
int threadPool_free(threadPool *pool)
{
    if (pool == NULL){
        return -1;
    }
    if (pool->task_queue){
        free(pool->task_queue);
    }
    if (pool->threads){
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool = NULL;
    return 0;
}
void *manager_thread(void *threadpool)
{
    int i;
    threadPool *pool = (threadPool *)threadpool;
    while (!pool->shutdown){
        sleep(DEFAULT_TIME);    //定时对线程池管理
        pthread_mutex_lock(&(pool->lock));
        int queue_size = pool->queue_size;     //关注任务数
        int live_thr_num = pool->live_thr_num; //存活线程数
        pthread_mutex_unlock(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        int busy_thr_num = pool->busy_thr_num; //忙着的线程数
        pthread_mutex_unlock(&(pool->thread_counter));
        //创建线程 算法:任务数大于最小线程池个数,且存活的线程数少于最大线程个数时,
        if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num){
            pthread_mutex_lock(&(pool->lock));
            int add = 0;
            //一次增加 DEFAULT_THREAD个线程
            for (i = 0; i < pool->max_thr_num && add<DEFAULT_THREAD_VARY && pool->live_thr_num <pool->max_thr_num ; i++) {
                if (pool->threads[i] == 0 || !is_thread_alive(pool->threads[i])){
                    pthread_create(&(pool->threads[i]),NULL,threadPool_thread,(void *)pool);
                    add++;
                    pool->live_thr_num++;
                }
            }
            pthread_mutex_unlock(&(pool->lock));
        }
        //销毁多余的空闲线程 算法:忙线程x2 小于存活线程数 且存活的线程数 大于 最小线程数时
        if((busy_thr_num *2)<live_thr_num && live_thr_num > pool->min_thr_num){
            //一次销毁DEFAULT_THREAD个线程,随机10个即可
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY; //要销毁的线程数 设置为10
            pthread_mutex_unlock(&(pool->lock));

            for (i = 0; i < DEFAULT_THREAD_VARY; i++) {
                //通知处在空闲状态的线程,他们会自行终止
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }
    return NULL;
}
//int threadPool_add(threadPool *pool,void*(*function)(void *arg),void *arg)
int threadPool_add(threadPool *pool,void*(*function)(void *arg),void *arg)
{
    pthread_mutex_lock(&(pool->lock));
    //== 为真,队列已经满,调wait阻塞
    while ((pool->queue_size == pool->queue_max_size)&&(!pool->shutdown)){
        pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
    }
    if (pool->shutdown){
        pthread_mutex_unlock(&(pool->lock));
    }
    //清空 工作线程 调用的回调函数 的参数arg*
    if (pool->task_queue[pool->queue_rear].arg != NULL){
        free(pool->task_queue[pool->queue_rear].arg);
        pool->task_queue[pool->queue_rear].arg = NULL;
    }
    //添加任务到任务队列里
    pool->task_queue[pool->queue_rear].function = function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear +1)%pool->queue_max_size; //队尾指针移动,模拟环形队列
    pool->queue_size++;
    //添加完任务后,队列不为空,唤醒线程中 等待处理任务的线程
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));
    return 0;
}
int threadPool_destroy(threadPool *pool)
{
    int i;
    if (pool == NULL){
        return -1;
    }
    pool->shutdown = true;
    //先销毁管理线程
    pthread_join(pool->manager_tid,NULL);
    for (i = 0; i <pool->live_thr_num ; i++) {
        //通知所有的空闲线程
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    for (i = 0; i < pool->live_thr_num; i++) {
        pthread_join(pool->threads[i],NULL);
    }
    threadPool_free(pool);
    return 0;
}
int is_thread_alive(pthread_t tid)
{
    int kill_rc = pthread_kill(tid,0);//发0号信号,测试线程是否存活
    if (kill_rc == ESRCH){
        return false;
    }
    return true;
}

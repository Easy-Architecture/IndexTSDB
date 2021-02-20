//
// Created by maomao on 2021/2/2.
//
#include "workQueue.h"

#define SQL_CACHE_FILE "./sql_cache/dump.osz"

sql_cache::~sql_cache()
{
    pthread_mutex_lock(&pop_lock);
    if (pop_mem != NULL)
        free(pop_mem);
    pop_mem_length = 0;
    pthread_mutex_unlock(&pop_lock);
    pthread_mutex_destroy(&pop_lock);

    pthread_mutex_lock(&swap_lock);
    if (cache_mem != NULL)
        free(cache_mem);
    cache_mem_length = 0;
    pthread_mutex_unlock(&swap_lock);
    pthread_mutex_destroy(&swap_lock);
    pthread_cond_destroy(&pop_fill);
    pthread_cond_destroy(&cache_clear);
}

int sql_cache::init()
{
    mmap_buf_offset = 0;

    pop_mem = (uint8_t *)malloc(max_push_number);
    if (pop_mem == NULL)
    {
        perror("fail to malloc pop mem\n");
        return -1;
    }
    memset(pop_mem, 0, max_push_number);
    pop_mem_offset = 0;
    pop_mem_length = 0;

    cache_mem = (uint8_t *)malloc(max_push_number);
    if (cache_mem == NULL)
    {
        perror("fail to malloc cache mem\n");
        return -1;
    }
    memset(cache_mem, 0, max_push_number);
    cache_mem_length = 0;

    if (pthread_mutex_init(&pop_lock, NULL))
    {
        perror("fail to create mutex\n");
        return -1;
    }
    if (pthread_mutex_init(&swap_lock, NULL))
    {
        perror("fail to create mutex\n");
        return -1;
    }
    if (pthread_cond_init(&pop_fill, NULL))
    {
        perror("fail to create cond\n");
        return -1;
    }
    if (pthread_cond_init(&cache_clear, NULL))
    {
        perror("fail to create cond\n");
        return -1;
    }

    return 0;
}

int sql_cache::run()
{
    //  开启cache线程
    pthread_t cache_thread_tid;
    pthread_create(&cache_thread_tid, NULL, cache_thread, (void *)this);

    //  开启执行sql语句的线程
    pthread_t execute_sql_thread_tid;
    pthread_create(&execute_sql_thread_tid, NULL, execute_sql_thread, (void *)this);

    pthread_join(cache_thread_tid, NULL);
    pthread_join(execute_sql_thread_tid, NULL);

    return 0;
}

//  获取本次需要从共享内存读取数据的长度
uint32_t get_read_buf_len(uint8_t *mmap_buf, uint32_t mmap_buf_len, uint32_t mmap_buf_offset)
{
    uint32_t read_buf_len;
    if (mmap_buf_len < mmap_buf_offset)
    {
        //  如果读取共享内存的偏移大于写入的偏移，说明写入写到共享内存的最大值后又从头写了
        uint32_t zero_sql_len, sql_len = 0;
        read_buf_len = mmap_buf_len + max_mmap_size - mmap_buf_offset;
        memcpy(&zero_sql_len, mmap_buf + 2 * sizeof(mmap_buf_len), sizeof(zero_sql_len));
        //  当前读取的偏移加当前字符串长度，小于共享内存大小减偏移值为0处的SQL串长度，循环
        while (mmap_buf_offset + sql_len < max_mmap_size - zero_sql_len - sizeof(zero_sql_len) - 2 * sizeof(mmap_buf_len))
        {
            uint32_t str_len;
            memcpy(&str_len, mmap_buf + 2 * sizeof(mmap_buf_len) + mmap_buf_offset + sql_len, sizeof(str_len));
            //  如果加上本段SQL串的长度会大于队列的最大值，则返回当前值
            if (sql_len + sizeof(str_len) + str_len > max_push_number)
                return sql_len;
            sql_len += sizeof(str_len) + str_len;
        }
        read_buf_len = mmap_buf_len + sql_len;
    }
    else if (mmap_buf_len - mmap_buf_offset > max_push_number)
    {
        //  如果读取共享内存的偏移大于写入的偏移，说明写入写到共享内存的最大值后又从头写了
        read_buf_len = 0;
        while (1)
        {
            uint32_t str_len;
            memcpy(&str_len, mmap_buf + 2 * sizeof(mmap_buf_len) + mmap_buf_offset + read_buf_len, sizeof(str_len));
            if (read_buf_len + sizeof(str_len) + str_len > max_push_number)
                break;
            read_buf_len += sizeof(str_len) + str_len;
        }
    }
    else
        read_buf_len = mmap_buf_len - mmap_buf_offset;

    return read_buf_len;
}

//  cache线程
void *cache_thread(void *arg)
{
    sql_cache *pcache = (sql_cache *)arg;
    while (1)
    {
        uint32_t mmap_buf_len;

        int fd = open(SQL_CACHE_FILE, O_RDWR, 00777);
        if (fd == -1)
        {
            perror("fail to open mmap file\n");
            exit(1);
        }
        struct stat sb;
        if (fstat(fd, &sb) == -1)
        {
            perror("stat mmap file fail\n");
            exit(1);
        }
        pcache->mmap_buf = (uint8_t *)mmap(NULL, max_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (pcache->mmap_buf == MAP_FAILED)
        {
            perror("fail to mmap\n");
            exit(1);
        }
        close(fd);

        //  先读取写入SQL到共享内存的程序的写入偏移
        memcpy(&mmap_buf_len, pcache->mmap_buf, sizeof(mmap_buf_len));
        if ((mmap_buf_len == 0) || (mmap_buf_len == pcache->mmap_buf_offset))
        {
            //  如果当前的写入长度为0，或者写入长度与读取偏移相等（即没有新数据），则等待填充数据
            usleep(empty_wait);
            munmap(pcache->mmap_buf, max_mmap_size);
            continue;
        }

        //  获取本次需要从共享内存读取数据的长度
        uint32_t read_buf_len = get_read_buf_len(pcache->mmap_buf, mmap_buf_len, pcache->mmap_buf_offset);
        if (pcache->pop_mem_length < max_push_number - read_buf_len)
        {
            //  如果pop队列的长度，小于最大队列长度减本次读取的数据长度，即可以向pop队列填充数据
            uint8_t *sql_list_buf = (uint8_t *)malloc(read_buf_len);
            if (mmap_buf_len < pcache->mmap_buf_offset)
            {
                //  如果读取共享内存的偏移大于写入的偏移，则先复制当前读取偏移到共享内存边界的数据，然后再复制偏移为0到写入偏移的数据
                memcpy(sql_list_buf, pcache->mmap_buf + 2 * sizeof(mmap_buf_len) + pcache->mmap_buf_offset, read_buf_len - mmap_buf_len);
                memcpy(sql_list_buf + read_buf_len - mmap_buf_len,  pcache->mmap_buf + 2 * sizeof(mmap_buf_len), mmap_buf_len);
            }
            else
                memcpy(sql_list_buf, pcache->mmap_buf + 2 * sizeof(mmap_buf_len) + pcache->mmap_buf_offset, read_buf_len);
            pthread_mutex_lock(&pcache->pop_lock);
            //  复制读取数据到pop队列
            memcpy(pcache->pop_mem + pcache->pop_mem_length, sql_list_buf, read_buf_len);
            if ((pcache->pop_mem_length == 0) || (pcache->pop_mem_offset == pcache->pop_mem_length))
            {
                pcache->pop_mem_length += read_buf_len;
                //  如果当前pop队列的长度为0，或者pop队列的读取偏移等于pop队列的长度，说明pop队列为空，发送pop队列已经填充数据的信号
                pthread_cond_signal(&pcache->pop_fill);
            }
            else
                pcache->pop_mem_length += read_buf_len;
            pthread_mutex_unlock(&pcache->pop_lock);

            pcache->mmap_buf_offset = mmap_buf_len;
            //  将当前读取偏移写入共享内存
            memcpy(pcache->mmap_buf + sizeof(mmap_buf_len), &pcache->mmap_buf_offset, sizeof(pcache->mmap_buf_offset));
            free(sql_list_buf);
        }
        else
        {
            uint8_t *sql_list_buf = (uint8_t *)malloc(read_buf_len);
            if (mmap_buf_len < pcache->mmap_buf_offset)
            {
                //  如果读取共享内存的偏移大于写入的偏移，则先复制当前读取偏移到共享内存边界的数据，然后再复制偏移为0到写入偏移的数据
                memcpy(sql_list_buf, pcache->mmap_buf + 2 * sizeof(mmap_buf_len) + pcache->mmap_buf_offset, read_buf_len - mmap_buf_len);
                memcpy(sql_list_buf + read_buf_len - mmap_buf_len,  pcache->mmap_buf + 2 * sizeof(mmap_buf_len), mmap_buf_len);
            }
            else
                memcpy(sql_list_buf, pcache->mmap_buf + 2 * sizeof(mmap_buf_len) + pcache->mmap_buf_offset, read_buf_len);
            pthread_mutex_lock(&pcache->swap_lock);
            //  如果cache队列的长度，大于队列最大值减当前读取数据的长度，则等待cache队列清空
            while (pcache->cache_mem_length > max_push_number - read_buf_len)
                pthread_cond_wait(&pcache->cache_clear, &pcache->swap_lock);
            //  复制读取数据到cache队列
            memcpy(pcache->cache_mem + pcache->cache_mem_length, sql_list_buf, read_buf_len);
            pcache->cache_mem_length += read_buf_len;
            pthread_mutex_unlock(&pcache->swap_lock);

            pcache->mmap_buf_offset = mmap_buf_len;
            //  将当前读取偏移写入共享内存
            memcpy(pcache->mmap_buf + sizeof(mmap_buf_len), &pcache->mmap_buf_offset, sizeof(pcache->mmap_buf_offset));
            free(sql_list_buf);
        }
        munmap(pcache->mmap_buf, max_mmap_size);
    }

    return NULL;
}

//  执行sql语句的线程
void *execute_sql_thread(void *arg)
{
    sql_cache *pcache = (sql_cache *)arg;
    while (1)
    {
        uint32_t str_len;
        char *sql_str;

        pthread_mutex_lock(&pcache->pop_lock);
        if ((pcache->pop_mem_length == 0) || (pcache->pop_mem_offset == pcache->pop_mem_length))
        {
            //  如果当前pop队列的长度为0，或者pop队列的读取偏移等于pop队列的长度，说明pop队列为空
            if (pcache->cache_mem_length != 0)
            {
                //  如果cache队列不为空，则交换pop队列与cache队列
                pthread_mutex_unlock(&pcache->pop_lock);
                pthread_mutex_lock(&pcache->swap_lock);
                uint8_t *p = pcache->cache_mem;
                pcache->cache_mem = pcache->pop_mem;
                pcache->pop_mem = p;
                pcache->pop_mem_offset = 0;
                pcache->pop_mem_length = pcache->cache_mem_length;
                pcache->cache_mem_length = 0;
                pthread_mutex_unlock(&pcache->swap_lock);
                pthread_cond_signal(&pcache->cache_clear);
                pthread_mutex_lock(&pcache->pop_lock);
            }
            else
            {
                //  如果cache队列也为空，等待pop队列已经填充数据的信号
                while ((pcache->pop_mem_length == 0) || (pcache->pop_mem_offset == pcache->pop_mem_length))
                    pthread_cond_wait(&pcache->pop_fill, &pcache->pop_lock);
            }
        }
        //  复制pop队列的数据
        memcpy(&str_len, pcache->pop_mem + pcache->pop_mem_offset, sizeof(str_len));
        sql_str = (char *)malloc(str_len);
        memcpy(sql_str, pcache->pop_mem + pcache->pop_mem_offset + sizeof(str_len), str_len);
        pcache->pop_mem_offset += str_len + sizeof(str_len);
        pthread_mutex_unlock(&pcache->pop_lock);

        //  执行SQL串，此处暂注释
        usleep(200 * 1000);
        printf("SQL len: %d\n", str_len);
        printf("SQL: %s\n", sql_str);
        free(sql_str);
    }

    return NULL;
}

int test(int argc, char *argv[])
{
    sql_cache sql_c;
    if (sql_c.init() == 0)
        sql_c.run();

    return 0;
}

#define SQL_CACHE_PATH "./sql_cache"
#define SQL_CACHE_FILE "./sql_cache/dump.osz"


const uint32_t max_mmap_size = 200 * 1000 * 1000;


string get_sql_buffer(string filename)
{
    ifstream fin(filename.c_str());
    char *buffer;
    fin.seekg(0, std::ios_base::end);
    uint64_t file_size = fin.tellg();
    buffer = (char *)malloc((size_t)(file_size + 1));
    fin.seekg(0, std::ios_base::beg);
    fin.read(buffer, file_size);
    fin.close();

    buffer[file_size] = 0;
    string str = buffer;
    free(buffer);

    return str;
}

int tests()
{
    string str = get_sql_buffer("./sql.log");

    if (access(SQL_CACHE_PATH, R_OK) == -1)
        mkdir(SQL_CACHE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    int fd = open(SQL_CACHE_FILE, O_CREAT | O_RDWR | O_TRUNC, 00777);
    if (fd == -1)
    {
        fprintf(stderr, "fail to open mmap file\n");
        exit(1);
    }
    lseek(fd, max_mmap_size - 1, SEEK_SET);
    write(fd, "", 1);

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        fprintf(stderr, "stat mmap file fail\n");
        exit(1);
    }
    uint8_t *mmap_buf = (uint8_t *)mmap(NULL, max_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (mmap_buf == MAP_FAILED)
    {
        fprintf(stderr, "fail to mmap\n");
        exit(1);
    }

    uint32_t mmap_buf_len = 0;
    string::size_type pos = str.find("\n");
    while (pos != string::npos)
    {
        string sql_str = str.substr(0, pos);
        str = str.substr(pos + 1, str.length() - pos - 1);
        pos = str.find("\n");
        //  先读取写入SQL到共享内存的程序的写入偏移
        uint32_t str_len = sql_str.length() + 1;
        memcpy(mmap_buf + 2 * sizeof(mmap_buf_len) + mmap_buf_len, &str_len, sizeof(str_len));
        memcpy(mmap_buf + 2 * sizeof(mmap_buf_len) + mmap_buf_len + sizeof(str_len),  sql_str.c_str(), str_len);
        if ((mmap_buf_len < 5000) || (str_len < 100))
            printf("SQL len: %d, %s\n", str_len, sql_str.c_str());
        mmap_buf_len += str_len + sizeof(str_len);
        memcpy(mmap_buf, &mmap_buf_len, sizeof(mmap_buf_len));
        printf("mmap_buf_len: %d\n", mmap_buf_len);
    }
    printf("str: %s\n", str.c_str());
    munmap(mmap_buf, max_mmap_size);

    return 0;
}

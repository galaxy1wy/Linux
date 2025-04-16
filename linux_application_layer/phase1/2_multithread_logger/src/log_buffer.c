/**
    @brief  日志缓冲区的实现
    @details 该文件包含日志缓冲区的定义，用于管理用于记录消息的环形缓冲区
    @details 该实现支持多线程环境下的读写操作
*/
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/log_buffer.h"

static atomic_uint_fast32_t global_log_id = 1; // 全局日志编号
static atomic_uint_fast32_t write_fail_count = 0; // 写失败次数

int log_buffer_init(log_buffer_t *buf)
{
    if(!buf)
        return -1;
    if (buf->magic != LOG_BUFFER_MAGIC || buf->version != LOG_BUFFER_VERSION) {
        buf->magic = LOG_BUFFER_MAGIC;
        buf->version = LOG_BUFFER_VERSION;
        atomic_store(&buf->head, 0);
        atomic_store(&buf->tail, 0);
        memset(buf->data, 0, BUFFER_SIZE);
        pthread_mutex_init(&buf->lock, NULL);
        pthread_cond_init(&buf->cond_can_read, NULL);
        pthread_cond_init(&buf->cond_can_write, NULL);
        return 1;  // 做了初始化
    }
    return 0;  // 已经初始化过
}

void log_buffer_destroy(log_buffer_t* buf)
{
    // sleep(1);
    pthread_mutex_lock(&buf->lock);
    pthread_cond_destroy(&buf->cond_can_write);
    pthread_cond_destroy(&buf->cond_can_read);
    pthread_mutex_unlock(&buf->lock);
    pthread_mutex_destroy(&buf->lock);
}

bool log_buffer_write(log_buffer_t *buf, const char *msg) {
    if (!buf || !msg) return false;
    if (buf->magic != LOG_BUFFER_MAGIC || buf->version != LOG_BUFFER_VERSION)
        return false;

    // 相当于 stanlen(msg,LOG_MESSAGE_MAX_LEN-1);
    size_t len;
    for (len = 0; len < LOG_MESSAGE_MAX_LEN-1 && msg[len]; len++);

    // 保证写入字符串后加换行符，占用 LOG_MESSAGE_MAX_LEN 字节
    size_t total = LOG_MESSAGE_MAX_LEN;
    // 添加编号前缀
    char temp_buf[LOG_MESSAGE_MAX_LEN] = {0};
    uint32_t log_id = atomic_fetch_add(&global_log_id, 1);
    int prefix_len = snprintf(temp_buf, sizeof(temp_buf), "[%u] ", log_id);
    size_t copy_len = (prefix_len + len > LOG_MESSAGE_MAX_LEN - 2) ? (LOG_MESSAGE_MAX_LEN - 2 - prefix_len) : len;
    memcpy(temp_buf + prefix_len, msg, copy_len);
    temp_buf[prefix_len + copy_len] = '\n';

    pthread_mutex_lock(&buf->lock);
    
    // 判断环形缓冲区是否已满：满的条件可以简单约定为 next_head == tail
    while (log_buffer_is_full(buf)) {
        pthread_cond_wait(&buf->cond_can_write, &buf->lock);
    }

    uint32_t head = atomic_load(&buf->head);
    uint32_t next_head = (head + total) % BUFFER_SIZE;
    
    // 如果未到缓冲区边界
    if (head + total <= BUFFER_SIZE) {
        memset(&buf->data[head], 0, total);
        memcpy(&buf->data[head], temp_buf, prefix_len + copy_len + 1);
    } else {
        size_t part1 = BUFFER_SIZE - head;
        size_t remain = prefix_len + copy_len + 1;
        memset(&buf->data[head], 0, part1);
        memcpy(&buf->data[head], temp_buf, part1 < remain ? part1 : remain);

        size_t part2 = total - part1;
        memset(&buf->data[0], 0, part2);
        if (remain > part1) {
            memcpy(&buf->data[0], temp_buf + part1, remain - part1);
        }
    }

    atomic_store(&buf->head, next_head);
    pthread_cond_signal(&buf->cond_can_read);    // 通知读线程有数据了
    pthread_mutex_unlock(&buf->lock);
    return true;
}

int log_buffer_read_batch(log_buffer_t *buf, char *out, size_t max_len)
{
    if (!buf || !out) return 0;
    if (buf->magic != LOG_BUFFER_MAGIC || buf->version != LOG_BUFFER_VERSION)
        return 0;

    pthread_mutex_lock(&buf->lock);
    
    if (log_buffer_is_empty(buf)) {
        pthread_cond_wait(&buf->cond_can_read, &buf->lock);
    }

    uint32_t tail = atomic_load(&buf->tail);
    uint32_t head = atomic_load(&buf->head);
    size_t count = 0;
    while (tail != head && count + LOG_MESSAGE_MAX_LEN <= max_len) {
        if (tail + LOG_MESSAGE_MAX_LEN <= BUFFER_SIZE) {
            memcpy(out + count, &buf->data[tail], LOG_MESSAGE_MAX_LEN);
        } else {
            size_t part1 = BUFFER_SIZE - tail;
            memcpy(out + count, &buf->data[tail], part1);
            memcpy(out + count + part1, &buf->data[0], LOG_MESSAGE_MAX_LEN - part1);
        }
        tail = (tail + LOG_MESSAGE_MAX_LEN) % BUFFER_SIZE;
        count += LOG_MESSAGE_MAX_LEN;
    }
    atomic_store(&buf->tail, tail);
    pthread_cond_broadcast(&buf->cond_can_write); // 通知所有写线程，有空位了
    pthread_mutex_unlock(&buf->lock);
    return count;
}

bool log_buffer_is_empty(log_buffer_t *buf)
{
    // 原子操作: 比较环形队列的 head 和 tail 指针的值，判断 head 和 tail 地址值是否相等
    return atomic_load(&buf->head) == atomic_load(&buf->tail);
}

bool log_buffer_is_full(log_buffer_t *buf)
{
    uint32_t head = atomic_load(&buf->head);
    uint32_t tail = atomic_load(&buf->tail);
    return ((head + LOG_MESSAGE_MAX_LEN) % BUFFER_SIZE) == tail;
}

uint32_t log_buffer_get_write_fail_count(void) {
    return atomic_load(&write_fail_count);
}

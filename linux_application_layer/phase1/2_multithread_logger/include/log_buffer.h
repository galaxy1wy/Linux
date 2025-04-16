/*
    * @file log_buffer.h
    * @brief 日志缓冲区实现的头文件
    * @details 该文件包含日志缓冲区的定义，用于管理用于记录消息的环形缓冲区
    * @author WY
    * @date 2025-04-11
    * @version 1.0
*/
#pragma once

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdatomic.h>

#define LOG_BUFFER_MAGIC    0x4C4F4742  // 'LOGB'
#define LOG_BUFFER_VERSION  1
#define LOG_MESSAGE_MAX_LEN 256         // 单条日志最大长度
#define BUFFER_SIZE         (1024 * 8)  // 日志条数（环形缓冲容量,必须为 LOG_MESSAGE_MAX_LEN 倍数）

// 环形缓冲区结构体
typedef struct{
    uint32_t magic;          // 用于判断是否已经初始化
    uint32_t version;        // 结构版本号
    atomic_uint head;        // 写指针：下一个写入的位置
    atomic_uint tail;        // 读指针：下一个读取的位置
    char data[BUFFER_SIZE];  // 环形缓冲区数据（每条占用 LOG_MESSAGE_MAX_LEN 字节）
    pthread_mutex_t lock;
    pthread_cond_t cond_can_read;
    pthread_cond_t cond_can_write;

}log_buffer_t;

/**
 * @brief 初始化日志缓冲区
 *
 * 只有当内存区域未被正确初始化（magic 或 version 不匹配）时，才会清空并初始化缓冲区，
 * 如果已初始化，则保留原有数据（实现崩溃后日志恢复）。
 *
 * @param buf 待初始化的缓冲区
 * @return int 0 表示已有数据，无需初始化；1 表示做了初始化； -1 表示错误
 */
int log_buffer_init(log_buffer_t *buf);

/*
 * @brief 销毁日志缓冲区buf
 * @param buf 日志缓冲区指针
 */
void log_buffer_destroy(log_buffer_t* buf);

/**
 * @brief 向日志缓冲区写入日志
 *
 * 将日志字符串写入缓冲区，如果空间不足则返回 false。写入的每条日志占用 LOG_MESSAGE_MAX_LEN 字节，
 * 格式为日志内容加换行符，不足部分清零。
 *
 * @param buf 日志缓冲区
 * @param msg 要写入的日志字符串
 * @return true 成功； false 失败（缓冲区满或参数错误）
 */
bool log_buffer_write(log_buffer_t* buf, const char* msg);

/**
 * @brief 批量读取日志数据
 *
 * 从环形缓冲区中批量读取日志数据到 out 缓冲区，最多读取 max_len 字节（必须为 LOG_MESSAGE_MAX_LEN 的倍数）。
 * 读取后会更新 tail 指针。
 *
 * @param buf 日志缓冲区
 * @param out 输出缓冲区
 * @param max_len 输出缓冲区大小
 * @return int 返回读取的字节数
 */
int log_buffer_read_batch(log_buffer_t *buf, char *out, size_t max_len);

// 判断缓冲区操作
bool log_buffer_is_empty(log_buffer_t* buf);
bool log_buffer_is_full(log_buffer_t* buf);
uint32_t log_buffer_get_write_fail_count(void);

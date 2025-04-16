/**
    @file disk_writer.c
    @brief 磁盘写入器实现
    @details 单独线程定期从 log_buffer 中批量读取日志并写入文件
    @details 支持配置批处理大小和刷新间隔
    @details 支持优雅关闭（graceful shutdown）
*/
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../include/disk_writer.h"

static void* disk_writer_thread(void *arg)
{
    disk_writer_t* writer = (disk_writer_t*)arg;
    char batch[BUFFER_SIZE]; // 临时缓冲区
    FILE* fp = fopen("persisted_log.txt", "a");
    if (!fp) {
        perror("fopen");
        return NULL;
    }
    while (writer->running) {
        memset(batch, 0, BUFFER_SIZE);
        int bytes = log_buffer_read_batch(writer->log_buffer, batch, sizeof(batch));
        if (bytes > 0) {
            for (int i = 0; i < bytes; i += LOG_MESSAGE_MAX_LEN) {
                int len = strnlen(batch + i, LOG_MESSAGE_MAX_LEN);
                if (len > 0) {
                    size_t written = fwrite(batch + i, 1, len, fp);
                    if (written != (size_t)len) {
                        perror("fwrite");
                    }
                }
            }
            fflush(fp);
            fsync(fileno(fp));
        }
    }
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    return NULL;
}

bool disk_writer_start(disk_writer_t* writer, log_buffer_t* buffer)
{
    if (!writer || !buffer) return false;
    writer->log_buffer = buffer;
    writer->running = true;
    return pthread_create(&writer->thread, NULL, disk_writer_thread, writer) == 0;
}

void disk_writer_stop(disk_writer_t* writer)
{
    if (!writer) return;
    writer->running = false;
    pthread_cond_broadcast(&writer->log_buffer->cond_can_read); // 唤醒以至于能退出
    pthread_join(writer->thread, NULL);
}
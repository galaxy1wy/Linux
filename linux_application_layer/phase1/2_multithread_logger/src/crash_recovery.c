/** 
    @file: crash_recovery.c
    @brief: 崩溃恢复模块实现
    @details: 该模块使用内存映射文件（mmap）来实现崩溃恢复
    @details: 该模块提供了初始化、清理、获取缓冲区和刷新缓冲区的功能
    @details: 该模块使用 POSIX 标准的文件操作函数
    @details: 该模块使用 mmap 来创建一个共享内存区域
    @details: 该模块使用 msync 来确保数据在崩溃后仍然可用

*/
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../include/crash_recovery.h"
#include "sys/types.h"


bool crash_recovery_init(crash_recovery_t *cr, const char *filepath, size_t size)
{
    if (!cr) return false;
    if (!filepath) filepath = DEFAULT_BACKING_FILE;

    cr->fd = open(filepath, O_RDWR | O_CREAT, 0644);
    if (cr->fd < 0){perror("{crash_recovery_init}open"); return false;}
    // 自动判定size是否符合要求
    size = (((size-BUFFER_SIZE)%LOG_MESSAGE_MAX_LEN == 0)&&(size>BUFFER_SIZE))?size:BUFFER_SIZE+LOG_MESSAGE_MAX_LEN;

    // 扩展文件大小以确保 mmap 空间足够
    if (ftruncate(cr->fd, size) != 0) {
        perror("{crash_recovery_init}ftruncate");
        close(cr->fd);
        return false;
    }

    cr->mapped_addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, cr->fd, 0);
    if (cr->mapped_addr == MAP_FAILED) {
        perror("{crash_recovery_init}mmap");
        close(cr->fd);
        return false;
    }

    cr->mapped_size = size;
    cr->log_buffer = (log_buffer_t*)cr->mapped_addr;

    // 如果不是有效的日志缓冲区，进行初始化
    if (cr->log_buffer->magic != LOG_BUFFER_MAGIC || cr->log_buffer->version != LOG_BUFFER_VERSION) {
        printf("日志缓冲区初始化\n");
        log_buffer_init(cr->log_buffer);
        // 写入新初始化的魔数和版本号
        cr->log_buffer->magic = LOG_BUFFER_MAGIC;
        cr->log_buffer->version = LOG_BUFFER_VERSION;
        // 强制刷新到磁盘
        msync(cr->mapped_addr, cr->mapped_size, MS_SYNC);
    }
    return true;
}

void crash_recovery_cleanup(crash_recovery_t *cr)
{
    if (!cr) return;
    if (cr->mapped_addr && cr->mapped_size) {
        msync(cr->mapped_addr, cr->mapped_size, MS_SYNC);
        munmap(cr->mapped_addr, cr->mapped_size);
    }
    if (cr->fd >= 0) close(cr->fd);
    cr->mapped_addr = NULL;
    cr->log_buffer = NULL;
    cr->mapped_size = 0;
    cr->fd = -1;
}

log_buffer_t* crash_recovery_get_buffer(crash_recovery_t *cr)
{
    return cr ? cr->log_buffer : NULL;
}

bool crash_recovery_flush(crash_recovery_t *cr)
{
    if (!cr || !cr->mapped_addr) return false;
    return msync(cr->mapped_addr, cr->mapped_size, MS_SYNC) == 0;
}
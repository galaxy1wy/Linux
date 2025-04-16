#include "../include/logger.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREAD_COUNT 5
#define MESSAGES_PER_THREAD 100

void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

void* writer_thread(void* arg) {
    int id = *(int*)arg;
    char msg[256];
    for (int i = 0; i < MESSAGES_PER_THREAD; i++) {
        snprintf(msg, sizeof(msg), "[Thread %d] Message %d", id, i);
        if(!logger_write(msg)){
            printf("日志{%s}未成功写入\n",msg);
        }
        // 使用 nanosleep 代替 usleep 更标准\n  struct timespec ts = {0, 10 * 1000 * 1000}; // 10ms\n   nanosleep(&ts, NULL);
        sleep_ms(10);   // 10ms
    }
    return NULL;
}

int main(void) {
    if (!logger_init("log_buffer.mmap", 1024*8)) {
        fprintf(stderr, "Logger initialization failed\n");
        return 1;
    }

    pthread_t threads[THREAD_COUNT];
    int ids[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, writer_thread, &ids[i]);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    // 手动刷新一次，确保所有日志持久化
    logger_flush();

    logger_shutdown();

    printf("程序结束\n");
    return 0;
}
#pragma once

#include <pthread.h>
#include <stdbool.h>
#include "log_buffer.h"

#define DEFAULT_BATCH_SIZE  16
#define DEFAULT_FLUSH_INTERVAL_MS   1000

typedef struct{
    pthread_t thread;               
    volatile bool running;
    log_buffer_t* log_buffer;
}disk_writer_t;

bool disk_writer_start(disk_writer_t* writer, log_buffer_t* buffer);
void disk_writer_stop(disk_writer_t* writer);
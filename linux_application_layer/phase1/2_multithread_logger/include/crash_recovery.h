#pragma once

#include "log_buffer.h"
#include <stdbool.h>
#include <stddef.h>

#define DEFAULT_BACKING_FILE    "log_buffer.mmap"

typedef struct{
    int fd;
    void *mapped_addr;
    size_t mapped_size;
    log_buffer_t *log_buffer;
}crash_recovery_t;

bool crash_recovery_init(crash_recovery_t *cr, const char *backing_file, size_t size);
void crash_recovery_cleanup(crash_recovery_t *cr);

log_buffer_t* crash_recovery_get_buffer(crash_recovery_t *cr);
bool crash_recovery_flush(crash_recovery_t *cr);

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdbool.h>
#include <stddef.h>

bool logger_init(const char* filepath, size_t buffer_size);
void logger_shutdown(void);
bool logger_write(const char* msg);
bool logger_flush(void);


#endif
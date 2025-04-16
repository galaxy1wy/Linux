
#include <stdio.h>
#include "../include/logger.h"
#include "../include/log_buffer.h"
#include "../include/disk_writer.h"
#include "../include/crash_recovery.h"


static crash_recovery_t g_cr;
static disk_writer_t g_writer;
static bool g_logger_initialized = false;


bool logger_init(const char* filepath, size_t buffer_size)
{
    if (g_logger_initialized) return true;

    if (!crash_recovery_init(&g_cr, filepath, buffer_size)) {
        fprintf(stderr, "Failed to initialize crash recovery\n");
        return false;
    }

    log_buffer_t* buf = crash_recovery_get_buffer(&g_cr);
    if (!disk_writer_start(&g_writer, buf)) {
        fprintf(stderr, "Failed to start disk writer\n");
        crash_recovery_cleanup(&g_cr);
        return false;
    }

    g_logger_initialized = true;
    return true;
}
void logger_shutdown(void)
{
    if (!g_logger_initialized) return;
    disk_writer_stop(&g_writer);
    log_buffer_destroy(g_cr.log_buffer);
    crash_recovery_cleanup(&g_cr);
    g_logger_initialized = false;
}
bool logger_write(const char* msg)
{
    if (!g_logger_initialized || !msg)  return false;
    return log_buffer_write(g_cr.log_buffer, msg);
}
bool logger_flush(void)
{
    if (!g_logger_initialized) return false;
    return crash_recovery_flush(&g_cr);
}
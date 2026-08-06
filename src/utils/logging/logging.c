#include "logging.h"
#include "utils/colours/colours.h"

static FILE* g_log_file = NULL;

void logging_init(FILE* file) {
    g_log_file = file;
}

void log_msg(LogLevel log_level, const char* const format, ...) {
    if (g_log_file == NULL) return;
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(g_log_file, "[%s] ", buffer);

    va_list argp;
    va_start(argp, format);
    switch (log_level) {
    case LOG_ERR:
        fprintf(g_log_file, TEXT_COLOR(ANSI_COLOR_RED, "[ERROR] "));
        break;
    case LOG_WARN:
        fprintf(g_log_file, TEXT_COLOR(ANSI_COLOR_YELLOW, "[WARNING] "));
        break;
    case LOG_INFO:
    default:
        fprintf(g_log_file, "[INFO] ");
        break;
    }

    vfprintf(g_log_file, format, argp);
    va_end(argp);
}
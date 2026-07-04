#include "logging.h"
#include "../colours/colours.h"

FILE* log_file = NULL;

void logging_init(FILE* file) {
    if (file != NULL) {
        log_file = file;
        return;
    } 
    log_file = stdout;
}

void log_msg(LogLevel log_level, const char* const format, ...) {
    time_t timer;
    char buffer[26];
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(log_file, "[%s] ", buffer);

    va_list argp;
    va_start(argp, format);
    switch (log_level) {
    case LOG_ERR:
        fprintf(log_file, TEXT_COLOR(ANSI_COLOR_RED, "[ERROR] "));
        break;
    case LOG_WARN:
        fprintf(log_file, TEXT_COLOR(ANSI_COLOR_YELLOW, "[WARNING] "));
        break;
    case LOG_INFO:
    default:
        fprintf(log_file, "[INFO] ");
        break;
    }

    vfprintf(log_file, format, argp);
    va_end(argp);
}

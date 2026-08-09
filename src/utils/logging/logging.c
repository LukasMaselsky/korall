#include "logging.h"
#include "utils/colours/colours.h"

// todo: can't log, race conditions

static FILE* g_log_file = NULL;

void logging_init(FILE* file) {
    g_log_file = file;
}

void log_msg(LogLevel log_level, const char* const format, ...) {
    if (g_log_file == NULL) return;
    
    const char* level = "";
    va_list argp;
    va_start(argp, format);
    switch (log_level) {
        case LOG_PLAIN:
            break;
        case LOG_ERR:
            level = TEXT_COLOR(ANSI_COLOR_RED, "[ERROR]");
            break;
        case LOG_WARN:
            level = TEXT_COLOR(ANSI_COLOR_YELLOW, "[WARNING]");
            break;
        case LOG_INFO:
            level = "[INFO]";
            break;
        default:
    }

    if (log_level != LOG_PLAIN) {
        time_t timer;
        char buffer[26] = { 0 };
        struct tm* tm_info;

        timer = time(NULL);
        tm_info = localtime(&timer);

        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(g_log_file, "[%s] %s ", buffer, level);
    }

    vfprintf(g_log_file, format, argp);
    va_end(argp);
}
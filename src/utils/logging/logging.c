#include "logging.h"
#include "utils/colours/colours.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

static FILE* g_log_file = NULL;

#ifdef _WIN32
static CRITICAL_SECTION log_cs;
#else
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void logging_init(FILE* file) {
    #ifdef _WIN32
    InitializeCriticalSection(&log_cs);
    #endif
    g_log_file = file;
}

void logging_cleanup() {
    #ifdef _WIN32
    DeleteCriticalSection(&log_cs);
    #endif
}

static void logging_lock() {
#ifdef _WIN32
    EnterCriticalSection(&log_cs);
#else
    pthread_mutex_lock(&log_mutex);
#endif
}

static void logging_unlock() {
#ifdef _WIN32
    LeaveCriticalSection(&log_cs);
#else
    pthread_mutex_unlock(&log_mutex);
#endif
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

    logging_lock();
    
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

    logging_unlock();
}
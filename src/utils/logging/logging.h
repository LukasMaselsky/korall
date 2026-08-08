#ifndef KORALL_LOGGING_H
#define KORALL_LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#ifndef KORALL_BUILD_TYPE_DISTRIBUTION
#define KORALL_LOG(log_level, format, ...) log_msg(log_level, format, ##__VA_ARGS__)
#define KORALL_LOG_DEBUG(format, ...) fprintf(stderr, "%s : %d : %s(): " format, __FILE__, __LINE__, __func__, ##__VA_ARGS__);
#else
#define KORALL_LOG(log_level, format, ...)
#define KORALL_LOG_DEBUG(format, ...)
#endif


typedef enum {
    LOG_DEBUG,
    LOG_ERR,
    LOG_WARN,
    LOG_INFO,
    LOG_PLAIN,
} LogLevel;

void logging_init(FILE* file);

void log_msg(LogLevel log_level, const char* const format, ...);



#endif
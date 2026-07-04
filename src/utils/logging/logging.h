#ifndef KORALL_LOGGING_H
#define KORALL_LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

typedef enum {
    LOG_DEBUG,
    LOG_ERR,
    LOG_WARN,
    LOG_INFO,
} LogLevel;

void logging_init(FILE* file);

void log_msg(LogLevel log_level, const char* const format, ...);



#endif
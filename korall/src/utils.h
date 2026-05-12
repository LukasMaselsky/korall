#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#ifdef _WIN32
    #include <WinSock2.h>
    #define MAX_FILE_PATH MAX_PATH
#else
    #include <linux/limits.h>
    #define MAX_FILE_PATH (PATH_MAX + 1)

#endif

#define KILOBYTE 1024
#define MEGABYTE (KILOBYTE * 1024)
#define GIGABYTE (MEGABYTE * 1024)

typedef enum {
    DAY_MON,
    DAY_TUE,
    DAY_WED,
    DAY_THU,
    DAY_FRI,
    DAY_SAT,
    DAY_SUN,
    DAY_COUNT
} Day;

typedef enum {
    MONTH_JAN,
    MONTH_FEB,
    MONTH_MAR,
    MONTH_APR,
    MONTH_MAY,
    MONTH_JUN,
    MONTH_JUL,
    MONTH_AUG,
    MONTH_SEP,
    MONTH_OCT,
    MONTH_NOV,
    MONTH_DEC,
    MONTH_COUNT
} Month;

typedef enum {
    STR_TO_INT_SUCCESS,
    STR_TO_INT_OVERFLOW,
    STR_TO_INT_UNDERFLOW,
    STR_TO_INT_INCONVERTIBLE
} str_to_int_errno;


typedef struct {
    char* chars;
    size_t size; // without null terminator
} String;

void memcpy_reverse(uint8_t* dest, const uint8_t* src, size_t size);

char* b64_encode(const unsigned char* in, size_t len);

str_to_int_errno str_to_int(int* out, const char* s, int base);

void int_to_str(const int value, char* str);

int strcmp_ci(const char* str1, const char* str2);

bool is_hex_digit(const char c);

bool is_digit(const char c);

bool is_digits_only(const char* str);

void* safe_calloc(size_t count, size_t size);

void get_current_time_gmt(struct tm** t);

bool starts_with(const char* pre, const char* str);

int fill_string_char(const char** str, char* arr, size_t arr_len, const char match);

int fill_string_str(const char** str, char* arr, size_t arr_len, const char *match, bool case_insensitive);


#endif
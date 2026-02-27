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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


typedef enum {
    STR_TO_INT_SUCCESS,
    STR_TO_INT_OVERFLOW,
    STR_TO_INT_UNDERFLOW,
    STR_TO_INT_INCONVERTIBLE
} str_to_int_errno;

typedef struct {
    int integer;
    char* string;
} LookupEntry;

str_to_int_errno str_to_int(int* out, char* s, int base);

void int_to_str(int value, char* str);

bool is_digit(const char c);

bool is_digits_only(const char* str);

int lookup_str_int(const char* key, const LookupEntry* table, const unsigned int table_count, const bool case_insensitive);

const char* lookup_int_str(const int key, const LookupEntry* table, const unsigned int table_count);

void* safe_calloc(size_t count, size_t size);

void get_current_time_gmt(struct tm** t);

#endif
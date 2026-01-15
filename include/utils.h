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


typedef enum {
    STR_TO_INT_SUCCESS,
    STR_TO_INT_OVERFLOW,
    STR_TO_INT_UNDERFLOW,
    STR_TO_INT_INCONVERTIBLE
} str_to_int_errno;

typedef struct {
    char* key;
    int val;
} LookupEntry;

str_to_int_errno str_to_int(int* out, char* s, int base);

bool is_digits_only(const char* str);

int lookup(char* key, LookupEntry* table, unsigned int table_count);

#endif
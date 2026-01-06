#ifndef UTILS_H
#define UTILS_H

typedef enum {
    STR_TO_INT_SUCCESS,
    STR_TO_INT_OVERFLOW,
    STR_TO_INT_UNDERFLOW,
    STR_TO_INT_INCONVERTIBLE
} str_to_int_errno;

str_to_int_errno str_to_int(int* out, char* s, int base);

int is_digits_only(const char* str);

#endif
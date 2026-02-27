#include "utils.h"
#ifndef _WIN32
#include <strings.h>
#endif

/* Convert string s to int out.
 *
 * @param[out] out The converted int. Cannot be NULL.
 *
 * @param[in] s Input string to be converted.
 *
 *     The format is the same as strtol,
 *     except that the following are inconvertible:
 *
 *     - empty string
 *     - leading whitespace
 *     - any trailing characters that are not part of the number
 *
 *     Cannot be NULL.
 *
 * @param[in] base Base to interpret string in. Same range as strtol (2 to 36).
 *
 * @return Indicates if the operation succeeded, or why it failed.
 */
str_to_int_errno str_to_int(int* out, char* s, int base) {
    char* end;
    if (s[0] == '\0' || isspace((unsigned char)s[0]))
        return STR_TO_INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR_TO_INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR_TO_INT_UNDERFLOW;
    if (*end != '\0')
        return STR_TO_INT_INCONVERTIBLE;
    *out = l;
    return STR_TO_INT_SUCCESS;
}

void int_to_str(int value, char* str) {
    sprintf(str, "%d", value);
}

bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}


bool is_digits_only(const char* str) {
    // strspn returns the length of the initial segment of str consisting only of characters in 0123456789
    if (str[0] == '\0') return false;
    return strspn(str, "0123456789") == strlen(str);
}

static int strcmp_ci(const char* str1, const char* str2) {
#ifdef _WIN32
    return stricmp(str1, str2);
#else
    return strcasecmp(str1, str2);
#endif
}

int lookup_str_int(const char* key, const LookupEntry *table, const unsigned int table_count, const bool case_insensitive) {
    if (key[0] == '\0') return -1;
    for (LookupEntry* entry = table; entry != table + table_count; entry++) {
        if (*(entry->string) == *key) {
            if (case_insensitive) {
                if (strcmp_ci(entry->string, key) == 0)
                    return entry->integer;
            }
            else {
                if (strcmp(entry->string, key) == 0)
                    return entry->integer;
            }
        }
        
    }

    return -1;
}

const char* lookup_int_str(const int key, const LookupEntry* table, const unsigned int table_count) {
    for (LookupEntry* entry = table; entry != table + table_count; entry++) {
        if (entry->integer == key)
            return entry->string;
    }

    return NULL;
}

void* safe_calloc(size_t count, size_t size) {
    void* p = calloc(count, size);
    if (p == NULL) {
        fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return p;
}

void get_current_time_gmt(struct tm **t) {
    time_t raw_time;
    time(&raw_time);
    *t = gmtime(&raw_time);
    return;
}
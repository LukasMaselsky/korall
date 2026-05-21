#include "utils.h"
#ifndef _WIN32
#include <strings.h>
#endif

void memcpy_reverse(uint8_t *dest, const uint8_t *src, size_t size) {
    
    for (int i = 0; i < size; i++) {
        dest[i] = src[size - i - 1];
    }
}

static size_t b64_encoded_size(size_t inlen)
{
    size_t ret;

    ret = inlen;
    if (inlen % 3 != 0)
        ret += 3 - (inlen % 3);
    ret /= 3;
    ret *= 4;

    return ret;
}

char* b64_encode(const unsigned char* in, size_t len)
{
    const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* out;
    size_t elen;
    size_t i;
    size_t j;
    size_t v;

    if (in == NULL || len == 0)
        return NULL;

    elen = b64_encoded_size(len);
    out = safe_calloc(elen + 1, 1);
    out[elen] = '\0';

    for (i = 0, j = 0; i < len; i += 3, j += 4) {
        v = in[i];
        v = i + 1 < len ? v << 8 | in[i + 1] : v << 8;
        v = i + 2 < len ? v << 8 | in[i + 2] : v << 8;

        out[j] = b64chars[(v >> 18) & 0x3F];
        out[j + 1] = b64chars[(v >> 12) & 0x3F];
        if (i + 1 < len) {
            out[j + 2] = b64chars[(v >> 6) & 0x3F];
        }
        else {
            out[j + 2] = '=';
        }
        if (i + 2 < len) {
            out[j + 3] = b64chars[v & 0x3F];
        }
        else {
            out[j + 3] = '=';
        }
    }

    return out;
}


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
str_to_int_errno str_to_int(int* out, const char* s, int base) {
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

void int_to_str(const int value, char* str) {
    sprintf(str, "%d", value);
}

bool is_hex_digit(const char c) {
    if (c <= 'f' && c >= 'a') return true;
    if (c <= 'F' && c >= 'A') return true;
    if (c <= '9' && c >= '0') return true;
    return false;
}

bool is_digit(const char c) {
    return c >= '0' && c <= '9';
}


bool is_digits_only(const char* str) {
    // strspn returns the length of the initial segment of str consisting only of characters in 0123456789
    if (str[0] == '\0') return false;
    return strspn(str, "0123456789") == strlen(str);
}

int strcmp_ci(const char* str1, const char* str2) {
#ifdef _WIN32
    return stricmp(str1, str2);
#else
    return strcasecmp(str1, str2);
#endif
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

bool starts_with(const char* pre, const char* str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

static int fill_str(const char **str, const char* s, const char *end, char* arr, size_t arr_len) {
    if (arr != NULL) {
        const size_t sub_len = end - s;
        if (sub_len > arr_len) return -1;
        memcpy(arr, s, sub_len);
        arr[sub_len] = '\0';
    }
    *str = end;
    return 0;
}

/*
    Fills char array with substring based on matching
*/
int fill_string_char(const char **str, char *arr, size_t arr_len, const char match) {
    const char* s = *str;
    const char* end = strchr(s, match);
    if (end == NULL) return -1;

    return fill_str(str, s, end, arr, arr_len);
}

static char* strlwr (char* s) {
    for (int i = 0; i < strlen(s); ++i)
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] += 'a' - 'A';
    return s;
}

int fill_string_str(const char** str, char* arr, size_t arr_len, const char *match, bool case_insensitive) {
    const char* s = *str;
    const char* t = match;
    if (case_insensitive) {
        s = strlwr(s);
        t = strlwr(t);
    }
    const char* end = strstr(s, t);
    if (end == NULL) return -1;

    return fill_str(str, s, end, arr, arr_len);
}
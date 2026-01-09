#include "utils.h"


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


int is_digits_only(const char* str) {
    // strspn returns the length of the initial segment of str consisting only of characters in 0123456789
    return strspn(str, "0123456789") == strlen(str);
}

#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "utils.h"
#include "server/main.h"

#define MIN_FLAG_CHAR_LEN 3 // --x
#define FLAG_LOOKUP_TABLE_COUNT F_COUNT
#define HTTP_METHOD_LOOKUP_TABLE_COUNT HTTP_METHOD_COUNT
#define HTTP_HEADER_FIELD_TABLE_COUNT HTTP_H_COUNT

extern const LookupEntry flag_lookup_table[FLAG_LOOKUP_TABLE_COUNT];
extern const LookupEntry http_method_lookup_table[HTTP_METHOD_LOOKUP_TABLE_COUNT];
extern const LookupEntry http_header_field_lookup_table[HTTP_HEADER_FIELD_TABLE_COUNT];
extern Flags default_flags;
#endif
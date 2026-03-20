#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "utils.h"
#include "http_internal.h"

#define HTTP_METHOD_TABLE_COUNT HTTP_METHOD_COUNT
#define HTTP_REQ_HEADER_FIELD_TABLE_COUNT HTTP_RQH_COUNT
#define HTTP_RES_HEADER_FIELD_TABLE_COUNT HTTP_RSH_COUNT
#define HTTP_STATUS_CODE_TABLE_COUNT HTTP_SC_COUNT
#define HTTP_MEDIA_TYPE_TABLE_COUNT HTTP_MT_COUNT
#define HTTP_ENCODING_TABLE_COUNT HTTP_ENC_COUNT
#define HTTP_TE_TABLE_COUNT HTTP_TE_COUNT
#define HTTP_TRANSFER_ENCODING_TABLE_COUNT HTTP_TRENC_COUNT
#define HTTP_CHARSET_TABLE_COUNT HTTP_CHS_COUNT
#define HTTP_CONNECTION_TABLE_COUNT HTTP_CON_COUNT
#define HTTP_REQ_CACHE_CONTROL_TABLE_COUNT HTTP_REQ_CC_COUNT
#define HTTP_RES_CACHE_CONTROL_TABLE_COUNT HTTP_RES_CC_COUNT
#define DAY_TABLE_COUNT DAY_COUNT
#define MONTH_TABLE_COUNT MONTH_COUNT

typedef struct {
    int integer;
    char* string;
} LookupEntry;

typedef struct {
    const LookupEntry* entries;
    const size_t size;
} LookupTable;

int lookup_str_int(const char* key, const LookupTable* table, const bool case_insensitive);

const char* lookup_int_str(const int key, const LookupTable* table);

extern const LookupTable http_method_lookup_table;
extern const LookupTable http_req_header_field_lookup_table;
extern const LookupTable http_res_header_field_lookup_table;
extern const LookupTable http_status_code_lookup_table;
extern const LookupTable http_media_type_lookup_table;
extern const LookupTable http_encoding_lookup_table;
extern const LookupTable http_te_lookup_table;
extern const LookupTable http_transfer_encoding_lookup_table;
extern const LookupTable http_charset_lookup_table;
extern const LookupTable http_connection_lookup_table;
extern const LookupTable http_req_cache_control_lookup_table;
extern const LookupTable http_res_cache_control_lookup_table;
extern const LookupTable day_lookup_table;
extern const LookupTable month_lookup_table;

#endif
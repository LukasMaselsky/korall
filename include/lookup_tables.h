#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "utils.h"
#include "server/main.h"

#define MIN_FLAG_CHAR_LEN 3 // --x
#define FLAG_LOOKUP_TABLE_COUNT F_COUNT
#define HTTP_METHOD_LOOKUP_TABLE_COUNT HTTP_METHOD_COUNT
#define HTTP_REQ_HEADER_FIELD_TABLE_COUNT HTTP_RQH_COUNT
#define HTTP_RES_HEADER_FIELD_TABLE_COUNT HTTP_RSH_COUNT
#define HTTP_STATUS_CODE_TABLE_COUNT HTTP_SC_COUNT

extern const LookupEntryStrInt flag_lookup_table[FLAG_LOOKUP_TABLE_COUNT];
extern const LookupEntryStrInt http_method_lookup_table[HTTP_METHOD_LOOKUP_TABLE_COUNT];
extern const LookupEntryStrInt http_req_header_field_lookup_table[HTTP_REQ_HEADER_FIELD_TABLE_COUNT];
extern const LookupEntryIntStr http_res_header_field_lookup_table[HTTP_RES_HEADER_FIELD_TABLE_COUNT];
extern const LookupEntryIntStr http_status_code_lookup_table[HTTP_STATUS_CODE_TABLE_COUNT];
extern Flags default_flags;
#endif
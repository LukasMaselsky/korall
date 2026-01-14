#include "utils.h"
#include "server/main.h"

#define MIN_FLAG_CHAR_LEN 3
#define FLAG_LOOKUP_TABLE_COUNT 2
#define HTTP_METHOD_LOOKUP_TABLE_COUNT 9

extern LookupEntry flag_lookup_table[FLAG_LOOKUP_TABLE_COUNT];
extern LookupEntry http_method_lookup_table[HTTP_METHOD_LOOKUP_TABLE_COUNT];
extern Flags default_flags;
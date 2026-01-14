#ifndef HTTP__H
#define HTTP__H
#include "utils.h"

#define MAX_HTTP_METHOD_STR_LEN 8

typedef enum {
	HTTP_CONNECT,
	HTTP_DELETE,
	HTTP_GET,
	HTTP_HEAD,
	HTTP_OPTIONS,
	HTTP_PATCH,
	HTTP_POST,
	HTTP_PUT,
	HTTP_TRACE,
	HTTP_BADMETHOD
} HTTPMethod;

HTTPMethod process_http_method(char* str, LookupEntry* table, int table_len);


#endif
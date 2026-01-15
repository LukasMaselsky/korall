#ifndef HTTP__H
#define HTTP__H
#include "utils.h"

#define MAX_HTTP_METHOD_STR_LEN 8
#define MAX_HTTP_QUERY_STR_LEN 1024
#define MAX_HTTP_URL_LEN 2048 // incl query str: https://stackoverflow.com/questions/812925/what-is-the-maximum-possible-length-of-a-query-string/48230425#48230425
#define HTTP_PROT_LEN 9

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

HTTPMethod process_http_method(const char** str, const LookupEntry* table, const int table_len);

int process_http_request_target(const char** str, HTTPMethod method);

int process_http_protocol(const char** str);


#endif
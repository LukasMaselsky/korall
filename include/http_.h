#ifndef HTTP__H
#define HTTP__H
#include "utils.h"

#define MAX_HTTP_METHOD_STR_LEN 8
#define MAX_HTTP_QUERY_STR_LEN 1025
#define MAX_HTTP_URL_LEN 2049 // incl query str: https://stackoverflow.com/questions/812925/what-is-the-maximum-possible-length-of-a-query-string/48230425#48230425
#define HTTP_PROT_LEN 9
// https://stackoverflow.com/questions/161738/what-is-the-best-regular-expression-to-check-if-a-string-is-a-valid-url#comment117272662_55468411

typedef enum {
	HTTP_BADMETHOD = -1,
	HTTP_CONNECT,
	HTTP_DELETE,
	HTTP_GET,
	HTTP_HEAD,
	HTTP_OPTIONS,
	HTTP_PATCH,
	HTTP_POST,
	HTTP_PUT,
	HTTP_TRACE
} HTTPMethod;

typedef struct {
	HTTPMethod method;
	char* request_target;
} HTTPRequest;

int validate_http_request(const char* data, int data_len, HTTPRequest* req);

HTTPMethod process_http_method(const char** str, const LookupEntry* table, const int table_len);

int process_http_request_target_relative(const char** str, HTTPRequest* req);

int process_http_request_target(const char** str, HTTPMethod method, HTTPRequest* req);

int process_http_protocol(const char** str);


#endif
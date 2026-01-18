#ifndef HTTP__H
#define HTTP__H
#include "utils.h"

#define MAX_HTTP_METHOD_STR_LEN 7
#define MAX_HTTP_QUERY_STR_LEN 1024
#define MAX_HTTP_URL_LEN 2048 // incl query str: https://stackoverflow.com/questions/812925/what-is-the-maximum-possible-length-of-a-query-string/48230425#48230425
#define HTTP_PROT_LEN 8
#define MAX_DOMAIN_LEN 253
#define MAX_HTTP_BODY_LEN 1000000 // 1mb?
#define MAX_HTTP_HEADER_FIELD_LEN 32
#define MAX_HTTP_HEADER_VALUE_LEN 4096 // cookie? // https://stackoverflow.com/questions/640938/what-is-the-maximum-size-of-a-web-browsers-cookies-key
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

typedef enum {
	HTTP_H_BADFIELD = -1,
	HTTP_H_HOST,
} HTTPHeaderField;

typedef struct {
	char* request_target;
	HTTPMethod method;
} HTTPRequestStartLine;


typedef struct {
	char* domain;
	char* port;
} HTTPHeaderHost;

typedef struct {
	HTTPHeaderHost *host;
} HTTPRequestHeaders;


typedef struct {
	char* body;
} HTTPRequestBody;

typedef struct {
	HTTPRequestStartLine *start_line;
	HTTPRequestHeaders* headers;
	HTTPRequestBody* body;
} HTTPRequest;

int validate_http_request(const char* data, int data_len, HTTPRequest* req);



int process_http_header_value(HTTPHeaderField field, char* value);

int process_http_header(const char** str, HTTPRequest* req);

int process_http_headers(const char** str, HTTPRequest* req);

HTTPMethod process_http_method(const char** str, const LookupEntry* table, const int table_len);

int process_http_request_target_relative(const char** str, HTTPRequest* req);

int process_http_request_target(const char** str, HTTPRequest* req);

int process_http_protocol(const char** str);

HTTPRequest* http_request_st_init();

void http_request_st_free(HTTPRequest* req);

void http_request_st_clear(HTTPRequest** req);



#endif
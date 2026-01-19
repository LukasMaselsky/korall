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
	HTTP_TRACE,
	HTTP_METHOD_COUNT
} HTTPMethod;

typedef enum {
	HTTP_H_BADFIELD = -1,
	HTTP_H_A_IM,
	HTTP_H_ACCEPT,
	HTTP_H_ACCEPT_CHARSET,
	HTTP_H_ACCEPT_DATETIME,
	HTTP_H_ACCEPT_ENCODING,
	HTTP_H_ACCEPT_LANGUAGE,
	HTTP_H_ACCESS_CONTROL_REQUEST_METHOD,
	HTTP_H_ACCESS_CONTROL_REQUEST_HEADERS,
	HTTP_H_AUTHORIZATION,
	HTTP_H_CACHECONTROL,
	HTTP_H_CONNECTION,
	HTTP_H_CONTENT_ENCODING,
	HTTP_H_CONTENT_LENGTH,
	HTTP_H_CONTENT_MD5,
	HTTP_H_CONTENT_TYPE,
	// HTTP_H_COOKIE,
	HTTP_H_DATE,
	HTTP_H_EXPECT,
	HTTP_H_FORWARDED,
	HTTP_H_FROM,
	HTTP_H_HOST,
	HTTP_H_HTTP2_SETTINGS,
	HTTP_H_IF_MATCH,
	HTTP_H_IF_MODIFIED_SINCE,
	HTTP_H_IF_NONE_MATCH,
	HTTP_H_IF_RANGE,
	HTTP_H_IF_UNMODIFIED_SINCE,
	HTTP_H_MAX_FORWARDS,
	HTTP_H_ORIGIN,
	HTTP_H_PRAGMA,
	HTTP_H_PREFER,
	HTTP_H_PROXY_AUTHORIZATION,
	HTTP_H_RANGE,
	HTTP_H_REFERER,
	HTTP_H_TE,
	HTTP_H_TRAILER,
	HTTP_H_TRANSFER_ENCODING,
	HTTP_H_USER_AGENT,
	HTTP_H_UPGRADE,
	HTTP_H_VIA,
	HTTP_H_WARNING,
	HTTP_H_COUNT
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



int process_http_header_value(const HTTPHeaderField field, const char* value, HTTPRequest* req);

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
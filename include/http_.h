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
#define MAX_DOMAIN_NAME_LEN 253
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

typedef enum {
	HTTP_SC_100 = 100,
	HTTP_SC_101 = 101,
	HTTP_SC_102 = 102,
	HTTP_SC_103 = 103,
	HTTP_SC_200 = 200,
	HTTP_SC_201 = 201,
	HTTP_SC_202 = 202,
	HTTP_SC_203 = 203,
	HTTP_SC_204 = 204,
	HTTP_SC_205 = 205,
	HTTP_SC_206 = 206,
	HTTP_SC_207 = 207,
	HTTP_SC_208 = 208,
	HTTP_SC_209 = 209,
	HTTP_SC_226 = 226,
	HTTP_SC_300 = 300,
	HTTP_SC_301 = 301,
	HTTP_SC_302 = 302,
	HTTP_SC_303 = 303,
	HTTP_SC_304 = 304,
	HTTP_SC_305 = 305,
	HTTP_SC_306 = 306,
	HTTP_SC_307 = 307,
	HTTP_SC_308 = 308,
	HTTP_SC_400 = 400,
	HTTP_SC_401 = 401,
	HTTP_SC_402 = 402,
	HTTP_SC_403 = 403,
	HTTP_SC_404 = 404,
	HTTP_SC_405 = 405,
	HTTP_SC_406 = 406,
	HTTP_SC_407 = 407,
	HTTP_SC_408 = 408,
	HTTP_SC_409 = 409,
	HTTP_SC_410 = 410,
	HTTP_SC_411 = 411,
	HTTP_SC_412 = 412,
	HTTP_SC_413 = 413,
	HTTP_SC_414 = 414,
	HTTP_SC_415 = 415,
	HTTP_SC_416 = 416,
	HTTP_SC_417 = 417,
	HTTP_SC_418 = 418,
	HTTP_SC_421 = 421,
	HTTP_SC_422 = 422,
	HTTP_SC_423 = 423,
	HTTP_SC_424 = 424,
	HTTP_SC_425 = 425,
	HTTP_SC_426 = 426,
	HTTP_SC_428 = 428,
	HTTP_SC_429 = 429,
	HTTP_SC_431 = 431,
	HTTP_SC_444 = 444,
	HTTP_SC_451 = 451,
	HTTP_SC_499 = 499,
	HTTP_SC_500 = 500,
	HTTP_SC_501 = 501,
	HTTP_SC_502 = 502,
	HTTP_SC_503 = 503,
	HTTP_SC_504 = 504,
	HTTP_SC_505 = 505,
	HTTP_SC_506 = 506,
	HTTP_SC_507 = 507,
	HTTP_SC_508 = 508,
	HTTP_SC_509 = 509,
	HTTP_SC_510 = 510,
	HTTP_SC_511 = 511,
	HTTP_SC_599 = 599,
	HTTP_SC_COUNT = 65,
} HTTPStatusCode;

typedef struct {
	char* body;
} HTTPBody;

// Request

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
	HTTPRequestStartLine *start_line;
	HTTPRequestHeaders* headers;
	HTTPBody* body;
} HTTPRequest;

// Response

typedef struct {
	HTTPStatusCode status_code;
	char *reason_phrase;
} HTTPResponseStartLine;

typedef struct {
	char* server;
} HTTPResponseHeaders;

typedef struct {
	HTTPResponseStartLine* start_line;
	HTTPResponseHeaders* headers;
	HTTPBody* body;
} HTTPResponse;

int validate_http_request(const char* data, int data_len, HTTPRequest* req);

int process_http_header_value(const HTTPHeaderField field, const char* value, HTTPRequest* req);

int process_http_header(const char** str, HTTPRequest* req);

int process_http_headers(const char** str, HTTPRequest* req);

HTTPMethod process_http_method(const char** str, const LookupEntryStrInt* table, const int table_len);

int process_http_request_target_relative(const char** str, HTTPRequest* req);

int process_http_request_target(const char** str, HTTPRequest* req);

int process_http_protocol(const char** str);

HTTPRequest* http_request_st_init();

void http_request_st_free(HTTPRequest* req);

void http_request_st_clear(HTTPRequest** req);



#endif
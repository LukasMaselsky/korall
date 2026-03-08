#ifndef HTTP_INTERNAL_H
#define HTTP_INTERNAL_H
#include "arena.h"
#include "array.h"
#include "sockets.h"
#include "utils.h"
#include "korall/http.h"

#define MAX_HTTP_METHOD_STR_LEN 7
#define MAX_HTTP_QUERY_STR_LEN 1024
#define MAX_HTTP_URL_LEN 2048 // incl query str: https://stackoverflow.com/questions/812925/what-is-the-maximum-possible-length-of-a-query-string/48230425#48230425
#define HTTP_PROT_LEN 8
#define MAX_DOMAIN_LEN 253
#define MAX_HTTP_BODY_LEN 1000000 // 1mb?
#define MAX_HTTP_HEADER_FIELD_LEN 32
#define MAX_HTTP_HEADER_VALUE_LEN 4096 // cookie? // https://stackoverflow.com/questions/640938/what-is-the-maximum-size-of-a-web-browsers-cookies-key
#define MAX_MEDIA_TYPE_LEN 74
#define MAX_HTTP_RES_LEN MAX_HTTP_BODY_LEN * 2
#define MAX_HTTP_BOUNDARY_LEN 70
#define MAX_ENCODING_CHAR_LEN 9
#define MAX_HTTP_CHARSET_LEN 13
#define MAX_HTTP_REQ_CC_LEN 15
#define MAX_HTTP_USER_AGENT (KILOBYTE * 4)

#define HTTP_RES_SIZE MAX_HTTP_BODY_LEN * 2
#define HTTP_REQ_SIZE HTTP_RES_SIZE

#define MAX_REASON_PHRASE_LEN 35

#define MAX_DATE_STR_LEN 29
// https://stackoverflow.com/questions/161738/what-is-the-best-regular-expression-to-check-if-a-string-is-a-valid-url#comment117272662_55468411

#define ERROR_MESSAGE(err, msg) \
    "{\n" \
    "\t\"error\": \"" err "\",\n" \
    "\t\"message\": \"" msg "\"\n" \
    "}"

#define HTTP_REQ_CC_HAS_VAL(x) (x == HTTP_REQ_CC_MAX_AGE || x == HTTP_REQ_CC_MAX_STALE || x == HTTP_REQ_CC_MIN_FRESH || x == HTTP_REQ_CC_STALE_IF_ERROR)

// Request

struct HTTPRequestStartLineInternal {
	char* request_target;
	HTTPMethod method;
};

// Headers

struct HTTPHeaderHostInternal {
	char* domain;
	char* port;
};

struct HTTPWeightedFieldInternal {
	int field;
	double weight;
};

struct HTTPContentTypeInternal {
	char* boundary;
	HTTPMediaType media_type;
	HTTPCharset charset;
};

struct HTTPRequestCacheControlPairInternal {
	HTTPRequestCacheControl name;
	int seconds;
};

struct HTTPDateInternal {
	Day day_name;
	Month month;
	unsigned short day;
	unsigned short year;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;
	bool used; // if this header used in req
};

struct HTTPRequestHeadersInternal {
	HTTPHeaderHost* host;
	Array* accept;
	Array* accept_encoding;
	int content_length;
	HTTPContentType* content_type;
	HTTPMethod access_control_request_method;
	Array* access_control_request_headers;
	HTTPConnection connection;
	Array* cache_control;
	char* user_agent;
	HTTPDate* date;
	HTTPExpect expect;
};

// Request

struct HTTPRequestInternal {
	HTTPRequestStartLine* start_line;
	HTTPRequestHeaders* headers;
	char* body;
};

// Response

struct HTTPResponseStartLineInternal {
	char* reason_phrase;
	HTTPStatusCode status_code;
};

struct HTTPResponseHeadersInternal {
	char* date;
	char* server;
	char* content_type;
	size_t content_length;
};


struct HTTPResponseInternal {
	HTTPResponseStartLine* start_line;
	HTTPResponseHeaders* headers;
	char* body;
};

HTTPError http_parse_request(const char* data, int data_len, HTTPRequest* req);

HTTPError http_process_header_value(const HTTPRequestHeaderField field, const char* value, HTTPRequest* req);

HTTPError http_process_header(const char** str, HTTPRequest* req);

HTTPError http_process_headers(const char** str, HTTPRequest* req);

HTTPError http_process_body(const char* str, HTTPRequest* req);

HTTPError http_process_method(const char** str, HTTPRequest* req);

HTTPError http_process_request_target_relative(const char** str, HTTPRequest* req);

HTTPError http_process_request_target(const char** str, HTTPRequest* req);

HTTPError http_process_protocol(const char** str);

HTTPRequest* http_request_init(Arena* arena);

HTTPRequestHeaders* http_request_init_headers(Arena* arena);

void http_request_free(Arena *arena, HTTPRequest* req);

void http_request_clear(Arena *arena, HTTPRequest** req);

HTTPResponse* http_response_construct(
	Arena* arena,
	HTTPStatusCode code,
	const char* server_name,
	HTTPMediaType content_type,
	const char* body
);

int http_header_to_str(HTTPResponseHeaderField field, const char* value, char** buf);

char* http_response_to_str(const HTTPResponse* res);

int http_response_send(const SOCKET inc_sock, const SOCKET server_sock, const HTTPResponse* res, const fd_set* main);

HTTPResponse* http_response_init(Arena *arena);

void http_response_free(Arena* arena, HTTPResponse* res);

void http_response_clear(Arena* arena, HTTPResponse** res);

void http_get_current_date(char* str, size_t str_len);

const char* http_error_response_info(HTTPError err, HTTPStatusCode* sc, HTTPMediaType* mt);

#endif
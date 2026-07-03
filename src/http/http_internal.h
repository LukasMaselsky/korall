#ifndef HTTP_INTERNAL_H
#define HTTP_INTERNAL_H
#include "arena/arena.h"
#include "socket/socket.h"
#include "utils/utils.h"
#include "config/config.h"
#include "korall/http.h"

#define WS_VERSION 13
#define WS_KEY_LEN 24
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_GUID_LEN 36

#define MAX_HTTP_METHOD_STR_LEN 7
#define MAX_HTTP_QUERY_STR_LEN 1024
#define MAX_HTTP_URL_LEN (2 * KILOBYTE) // incl query str: https://stackoverflow.com/questions/812925/what-is-the-maximum-possible-length-of-a-query-string/48230425#48230425
#define HTTP_PROT_LEN 8
#define MAX_DOMAIN_LEN 255
#define MAX_DOMAIN_LABEL_LEN 63
#define MIN_DOMAIN_LABEL_LEN 1
#define MAX_HTTP_BODY_LEN MEGABYTE // todo: 1mb?
#define MAX_HTTP_BODY_DIGIT_LEN 7 // 7 digits
#define MAX_HTTP_HEADER_FIELD_LEN 32
#define MAX_HTTP_HEADER_VALUE_LEN 4096 // todo: cookie? // https://stackoverflow.com/questions/640938/what-is-the-maximum-size-of-a-web-browsers-cookies-key
#define MAX_MEDIA_TYPE_LEN 73
#define MAX_HTTP_BOUNDARY_LEN 70
#define MAX_ACCEPT_ENCODING_CHAR_LEN 8
#define MAX_ACCEPT_LANGUAGE_CHAR_LEN 7
#define MAX_TE_LEN 8
#define MAX_TRANSFER_ENCODING_LEN 8
#define MAX_HTTP_CHARSET_LEN 12
#define MAX_HTTP_REQ_CC_LEN 14
#define MAX_HTTP_RES_CC_LEN 21
#define MAX_HTTP_USER_AGENT (4 * KILOBYTE)
#define MAX_REASON_PHRASE_LEN 34
#define MAX_CONTENT_ENCODING_CHAR_LEN 8

// request

#define HTTP_REQ_START_LINE_LEN (MAX_HTTP_METHOD_STR_LEN + 1 + MAX_HTTP_URL_LEN + 1 + HTTP_PROT_LEN + 2)
#define HTTP_REQ_HEADER_LEN (8 * KILOBYTE)
#define HTTP_REQ_HEADER_COUNT 100
#define HTTP_REQ_HEADERS_LEN (HTTP_REQ_HEADER_LEN * HTTP_REQ_HEADER_COUNT)
#define HTTP_REQ_BODY_LEN (1 * MEGABYTE) // todo: change to 1 GB?
#define HTTP_REQ_SIZE (HTTP_REQ_START_LINE_LEN + HTTP_REQ_HEADERS_LEN + HTTP_REQ_BODY_LEN)
#define HTTP_REQ_ARENA_SIZE (HTTP_REQ_SIZE + KILOBYTE)

// response

#define HTTP_RES_START_LINE_LEN (HTTP_PROT_LEN + 1 + 3 + MAX_REASON_PHRASE_LEN + 2)
#define HTTP_RES_HEADER_LEN (8 * KILOBYTE)
#define HTTP_RES_HEADER_COUNT 100
#define HTTP_RES_HEADERS_LEN (HTTP_RES_HEADER_LEN * HTTP_RES_HEADER_COUNT)
#define HTTP_RES_BODY_LEN (1 * MEGABYTE)
#define HTTP_RES_SIZE (HTTP_RES_START_LINE_LEN + HTTP_RES_HEADERS_LEN + HTTP_RES_BODY_LEN)
#define HTTP_RES_ARENA_SIZE (HTTP_RES_SIZE + KILOBYTE)
#define HTTP_RES_FULL_ARENA_SIZE HTTP_RES_SIZE


#define MAX_DATE_STR_LEN 29
// https://stackoverflow.com/questions/161738/what-is-the-best-regular-expression-to-check-if-a-string-is-a-valid-url#comment117272662_55468411

#define ERROR_MESSAGE(err, msg) \
    "{\n" \
    "\t\"error\": \"" err "\",\n" \
    "\t\"message\": \"" msg "\"\n" \
    "}"

#define HTTP_REQ_CC_HAS_VAL(x) (x == HTTP_REQ_CC_MAX_AGE || x == HTTP_REQ_CC_MAX_STALE || x == HTTP_REQ_CC_MIN_FRESH || x == HTTP_REQ_CC_STALE_IF_ERROR)
#define HTTP_RES_CC_HAS_VAL(x) (x == HTTP_RES_CC_MAX_AGE || x == HTTP_RES_CC_S_MAX_AGE || x == HTTP_RES_CC_STALE_WHILE_REVALIDATE || x == HTTP_RES_CC_STALE_IF_ERROR)

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

struct HTTPRequestWebsocketInternal {
	char* accept;
	bool has_connection;
	bool has_upgrade;
	bool has_key;
	bool has_version;
};

// Request

struct HTTPRequestInternal {
	HTTPRequestStartLine* start_line;
	HTTPHeaderHost* host;
	char* headers;
	char* body;
	HTTPRequestWebsocket* ws;
};

// Response


struct HTTPResponseInternal {
	String start_line;
	String headers;
	size_t header_count;
	size_t header_capacity;
	size_t header_size;
	char* headers_base;
	String body;
};

HTTPError http_request_parse(const char* data, HTTPRequest* req);

HTTPError http_process_request_header_value(const HTTPRequestHeaderField field, const char* value, HTTPRequest* req);

HTTPError http_request_process_header(const char** str, HTTPRequest* req);

HTTPError http_request_process_headers(const char** str, HTTPRequest* req);

HTTPError http_request_process_body(const char* str, HTTPRequest* req);

HTTPError http_request_process_method(const char** str, HTTPRequest* req);

HTTPError http_request_process_target_relative(const char* str, HTTPRequest* req);

HTTPError http_request_process_target_absolute(const char* str, HTTPRequest* req);

HTTPError http_request_process_target(const char** str, HTTPRequest* req);

HTTPError http_request_process_protocol(const char** str);

HTTPRequest* http_request_init(Arena* arena);

void http_request_free(Arena *arena);

void http_request_clear(Arena *arena, HTTPRequest** req);

// Response

HTTPError http_process_response_header_value(const HTTPResponseHeaderField field, const char* value);

int http_response_construct(
	HTTPResponse* res,
	HTTPStatusCode code,
	const char* server_name,
	HTTPMediaType content_type,
	const char* body
);

int http_response_ws_construct(
	HTTPResponse* res,
	const char* accept,
	const char* server_name
);

int http_response_send(const SOCKET inc_sock, const HTTPResponse* res);

HTTPResponse* http_response_init(Arena *arena);

void http_response_free(Arena* arena);

void http_response_clear(Arena* arena, HTTPResponse** res);

void http_get_current_date(String *str);

const char* http_error_response_info(HTTPError err, HTTPStatusCode* sc, HTTPMediaType* mt);

#endif
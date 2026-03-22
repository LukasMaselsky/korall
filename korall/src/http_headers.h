#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include "http_internal.h"

HTTPError http_domain_port(const char* value, char* domain, char* port, bool* with_port);

HTTPError http_process_host(const char* value, HTTPRequest* req);

HTTPError http_process_accept(const char* value);

HTTPError http_process_accept_encoding(const char* value);

HTTPError http_process_content_length(const char* value);

HTTPError http_process_content_type(const char* value);

HTTPError http_process_access_control_request_method(const char* value);

HTTPError http_process_access_control_request_headers(const char* value);

HTTPError http_process_connection(const char* value);

HTTPError http_process_cache_control_req(const char* value);

HTTPError http_process_user_agent(const char* value);

HTTPError http_process_date(const char* value);

HTTPError http_process_expect(const char* value);

HTTPError http_process_te(const char* value);

HTTPError http_process_transfer_encoding(const char* value);

HTTPError http_process_server(const char* value);

HTTPError http_process_max_forwards(const char* value);

HTTPError http_process_tk(const char* value);

/* Response */

HTTPError http_process_cache_control_res(const char* value);


#endif
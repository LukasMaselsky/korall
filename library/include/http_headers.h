#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include "http_.h"

HTTPError http_domain_port(const char* value, char* domain, char* port, bool* with_port);

HTTPError http_process_host(const char* value, HTTPRequest* req);

HTTPError http_process_accept(const char* value, HTTPRequest* req);

HTTPError http_process_accept_encoding(const char* value, HTTPRequest* req);

HTTPError http_process_content_length(const char* value, HTTPRequest* req);

HTTPError http_process_content_type(const char* value, HTTPRequest* req);

HTTPError http_process_access_control_request_method(const char* value, HTTPRequest* req);

HTTPError http_process_access_control_request_headers(const char* value, HTTPRequest* req);

HTTPError http_process_connection(const char* value, HTTPRequest* req);

HTTPError http_process_cache_control(const char* value, HTTPRequest* req);

HTTPError http_process_user_agent(const char* value, HTTPRequest* req);

HTTPError http_process_date(const char* value, HTTPRequest* req);

HTTPError http_process_expect(const char* value, HTTPRequest* req);

#endif
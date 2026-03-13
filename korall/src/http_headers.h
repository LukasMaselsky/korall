#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include "http_internal.h"

HTTPError http_domain_port(const char* value, char* domain, char* port, bool* with_port);

HTTPError http_process_host_req(const char* value, HTTPRequest* req);

HTTPError http_process_accept_req(const char* value, HTTPRequest* req);

HTTPError http_process_accept_encoding_req(const char* value, HTTPRequest* req);

HTTPError http_process_content_length_req(const char* value, HTTPRequest* req);

HTTPError http_process_content_type_req(const char* value, HTTPRequest* req);

HTTPError http_process_access_control_request_method_req(const char* value, HTTPRequest* req);

HTTPError http_process_access_control_request_headers_req(const char* value, HTTPRequest* req);

HTTPError http_process_connection_req(const char* value, HTTPRequest* req);

HTTPError http_process_cache_control_req(const char* value, HTTPRequest* req);

HTTPError http_process_user_agent_req(const char* value, HTTPRequest* req);

HTTPError http_process_date_req(const char* value, HTTPRequest* req);

HTTPError http_process_expect_req(const char* value, HTTPRequest* req);

/* Response */

HTTPError http_process_content_length_res(const char* value);

HTTPError http_process_content_type_res(const char* value);

HTTPError http_process_connection_res(const char* value);

HTTPError http_process_cache_control_res(const char* value);

HTTPError http_process_server_res(const char* value);

HTTPError http_process_date_res(const char* value);

#endif
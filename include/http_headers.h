#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include "http_.h"

int http_domain_port(const char* value, char* domain, char* port, bool* with_port);

int http_process_host(const char* value, HTTPRequest* req);

int http_process_accept(const char* value, HTTPRequest* req);

#endif
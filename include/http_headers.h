#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include "http_.h"

int http_domain_port(const char* value, char* domain, char* port, bool* with_port);

int process_http_host(const char* value, HTTPRequest* req);


#endif
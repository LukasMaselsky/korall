#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include "http_.h"

int http_domain_port(const char* value, char* domain, char* port, bool* with_port);

int http_process_host(const char* value, HTTPRequest* req);

int http_process_weighted_list(
	const char* value,
	HTTPRequest* req,
	LookupEntry* table,
	int table_len,
	char* field_arr,
	const int field_arr_len,
	Array* res_arr
);

int http_process_accept(const char* value, HTTPRequest* req);

int http_process_accept_encoding(const char* value, HTTPRequest* req);

int http_process_content_length(const char* value, HTTPRequest* req);

int http_process_content_type(const char* value, HTTPRequest* req);

#endif
#include "http_.h"
#include "utils.h"
#include "lookup_tables.h"

int process_http_protocol(const char** str) {
	char prot[HTTP_PROT_LEN] = { 0 };
	int len = HTTP_PROT_LEN;
	int i = 0;
	const char* s = *str;
	for (char c = *s; c != '\0' && i < len - 1; c = *(++s)) {
		prot[i] = c;
		i++;
	}
	prot[i] = '\0';

	if (strncmp("HTTP/1.1", prot, HTTP_PROT_LEN - 1) == 0) return 0;
	return -1;
}

bool is_valid_http_url(const char* str) {
	return false;
}

int process_http_request_target(const char **str, HTTPMethod method) {
	if (method == HTTP_OPTIONS) {
		const char* s = *str;
		if (s[0] == '*') {
			*str = ++s;
			return 0;
		}
	}
	if (method == HTTP_CONNECT) {

		return 0;
	}
	return -1;
}

HTTPMethod process_http_method(const char **str, const LookupEntry *table, const int table_len) {
	
	char method[MAX_HTTP_METHOD_STR_LEN] = { 0 };
	int len = MAX_HTTP_METHOD_STR_LEN;
	int i = 0;
	const char* s = *str;
	for (char c = *s; c != '\0' && c != ' ' && i < len - 1; c = *(++s)) {
		method[i] = c;
		i++;
	}
	method[i] = '\0';
	*str = s;
	return lookup(method, table, table_len);
}
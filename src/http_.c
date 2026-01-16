#include "http_.h"
#include "utils.h"
#include "lookup_tables.h"

int process_http_protocol(const char** str) {
	char prot[HTTP_PROT_LEN] = { 0 };
	int len = HTTP_PROT_LEN;
	int i = 0;
	const char* s = *str;
	for (char c = *s; c != '\0' && c != '\n' && i < len - 1; c = *(++s)) {
		prot[i] = c;
		i++;
	}
	prot[i] = '\0';
	*str = s;

	if (strncmp("HTTP/1.1", prot, HTTP_PROT_LEN - 1) == 0) return 0;
	return -1;
}

bool is_valid_http_url(const char* str) {
	return false;
}

bool is_valid_request_target_relative(char c) {
	// ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789
	// $%&'()*+,-./:;=[]_~
	// @!?#
	if (c <= 'z' && c >= 'a') return true;
	if (c <= 'Z' && c >= 'A') return true;
	if (c <= '9' && c >= '0') return true;
	if (c <= '/' && c >= '$') return true;
	if (c == ':' || c == ';' || c == '=' || c == '[' || c == ']' || c == '_' || c == '~') return true;
	return false;
}

int process_http_request_target_relative(const char** str, HTTPRequest* req) {
	// todo: query string
	int len = MAX_HTTP_URL_LEN;
	int i = 0;
	char* rt = req->request_target;
	const char* s = *str;
	for (char c = *s; c != '\0' && c != ' ' && i < len - 1; c = *(++s)) {
		if (!is_valid_request_target_relative(c)) return -1;
		if (i != 0) {
			if (c == '/' && *(s - 1) == '/') return -1;
		}
		rt[i] = c;
		i++;
	}
	rt[i] = '\0';
	*str = s;

	return 0;
}

int process_http_request_target(const char **str, HTTPMethod method, HTTPRequest *req) {
	if (method == HTTP_BADMETHOD) return -1;
	const char* s = *str;
	if (method == HTTP_OPTIONS && s[0] == '*') {
		(req->request_target)[0] = '*';
		(req->request_target)[1] = '\0';
		*str = ++s;
		return 0;
	}

	const char first_c = s[0];
	if (first_c == '/') {
		// relative path
		return process_http_request_target_relative(str, req);
	}
	else if (first_c == 'h' || first_c == 'H') {
		// absolute path
		// todo
		printf("Absolute paths not supported");
		return -1;
	}
	else {
		return -1;
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
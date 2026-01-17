#include "http_.h"
#include "utils.h"
#include "lookup_tables.h"
#include "sockets.h"

// host needed for relative rt
int validate_http_request(const char* data, int data_len, HTTPRequest* req) {
	const LookupEntry* table = &http_method_lookup_table;
	const int table_len = HTTP_METHOD_LOOKUP_TABLE_COUNT;

	HTTPMethod method = process_http_method(&data, table, table_len);
	if (method == HTTP_BADMETHOD || data[0] == '\0') return -1;
	req->start_line->method = method;
	if (*data != ' ') return -1;
	data++; // advance past space

	// process rt
	int res = process_http_request_target(&data, req);
	if (res == -1) return -1;
	if (*data != ' ') return -1;
	data++; // advance past space

	// process prot
	res = process_http_protocol(&data);
	if (res == -1) return -1;

	if (*data != '\n') return -1; // must have newline
	data++; // advance past newline

	if (*data == '\n' && data[1] == '\0') return 0; // no header, no body

	// todo: process headers and body

	return 0;
}

int process_http_protocol(const char** str) {
	char prot[HTTP_PROT_LEN + 1] = { 0 };
	int len = HTTP_PROT_LEN;
	int i = 0;
	const char* s = *str;
	for (char c = *s; c != '\0' && c != '\n' && i < len; c = *(++s)) {
		prot[i] = c;
		i++;
	}
	prot[i] = '\0';
	*str = s;

	if (strncmp("HTTP/1.1", prot, HTTP_PROT_LEN) == 0) return 0;
	return -1;
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
	char* rt = req->start_line->request_target;
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

int process_http_request_target(const char **str, HTTPRequest *req) {
	HTTPMethod method = req->start_line->method;
	if (method == HTTP_BADMETHOD) return -1;
	const char* s = *str;
	if (method == HTTP_OPTIONS && s[0] == '*') {
		(req->start_line->request_target)[0] = '*';
		(req->start_line->request_target)[1] = '\0';
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
	
	char method[MAX_HTTP_METHOD_STR_LEN + 1] = { 0 };
	int len = MAX_HTTP_METHOD_STR_LEN;
	int i = 0;
	const char* s = *str;
	for (char c = *s; c != '\0' && c != ' ' && i < len; c = *(++s)) {
		method[i] = c;
		i++;
	}
	method[i] = '\0';
	*str = s;
	return lookup(method, table, table_len);
}

HTTPRequest* http_request_st_init() {
	// start line
	char* request_target;
	request_target = (char*) safe_calloc(MAX_HTTP_URL_LEN + 1, sizeof(*request_target));

	HTTPRequestStartLine* sl;
	sl = (HTTPRequestStartLine*) safe_calloc(1, sizeof(*sl));
	sl->request_target = request_target;

	// headers

	char *domain, *port;
	domain = (char*)safe_calloc(MAX_DOMAIN_LEN + 1, sizeof(*domain));
	port = (char*)safe_calloc(MAX_PORT_NUM_CHAR_LEN + 1, sizeof(*port));

	HTTPHeaderHost* hh;
	hh = (HTTPHeaderHost*) safe_calloc(1, sizeof(*hh));
	hh->domain = domain;
	hh->port = port;

	HTTPRequestHeaders* headers;
	headers = (HTTPRequestHeaders*)safe_calloc(1, sizeof(*headers));
	headers->host = hh;

	// body

	char* body;
	body = (char*)safe_calloc(MAX_HTTP_BODY_LEN + 1, sizeof(char*));
	HTTPRequestBody* rq_body;
	rq_body = (HTTPRequestBody*)safe_calloc(1, sizeof(*rq_body));
	rq_body->body = body;

	// all

	HTTPRequest *req;
	req = (HTTPRequest*) safe_calloc(1, sizeof(*req));
	req->start_line = sl;
	req->headers = headers;
	req->body = rq_body;
	return req;
}

void http_request_st_free(HTTPRequest* req) {
	// start line

	free(req->start_line->request_target);
	free(req->start_line);

	// headers

	free(req->headers->host->domain);
	free(req->headers->host->port);
	free(req->headers->host);
	free(req->headers);

	// body

	free(req->body->body);
	free(req->body);

	// all

	free(req);
}

void http_request_st_clear(HTTPRequest** req) {
	http_request_st_free(*req);
	*req = http_request_st_init();
}
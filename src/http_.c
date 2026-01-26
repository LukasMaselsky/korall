#include "http_.h"
#include "utils.h"
#include "lookup_tables.h"
#include "sockets.h"
#include "http_headers.h"

/*
	Check if the format of the HTTP request is correct
*/
int validate_http_request(const char* data, int data_len, HTTPRequest* req) {
	const LookupEntryStrInt* table = &http_method_lookup_table;
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

	if (*data != '\r' || data[1] != '\n') return -1; // must have \r\n
	data += 2;

	if (*data == '\r' && data[1] == '\n' && data[2] == '\0') return 0; // no header, no body

	res = process_http_headers(&data, req);
	if (res == -1) return -1;

	// todo: body

	return 0;
}

int process_http_header_value(const HTTPRequestHeaderField field, const char* value, HTTPRequest *req) {
	// massive switch for each header
	switch (field) {
		case HTTP_RQH_HOST:
			return process_http_host(value, req);
			break;
		default:
			return -1; // todo: allow custom headers
	}
}

int process_http_header(const char** str, HTTPRequest* req) {
	// process field

	char field[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 };
	int len = MAX_HTTP_HEADER_FIELD_LEN;
	int i = 0;
	const char* s = *str;
	for (char c = *s; c != ':' && i < len; c = *(++s)) {
		if (c == '\0' || c == '\n') return -1;
		field[i] = c;
		i++;
	}
	field[i] = '\0';
	if (*s != ':') return -1; // field too long

	HTTPRequestHeaderField header_field = lookup_str_int(field, http_req_header_field_lookup_table, HTTP_REQ_HEADER_FIELD_TABLE_COUNT, true);
	if (header_field == HTTP_RQH_BADFIELD) return -1;

	s++; // skip colon
	// process value

	// skip through whitespace
	while (*s == ' ')
		s++;

	if (*s == '\0' || *s == '\n') return -1; // empty field e.g. "Host:    ";

	char value[MAX_HTTP_HEADER_VALUE_LEN + 1] = { 0 };
	len = MAX_HTTP_HEADER_FIELD_LEN;
	i = 0;
	for (char c = *s; c != '\r' && c != '\0' && i < len; c = *(++s)) {
		value[i] = c;
		i++;
	}
	value[i] = '\0';
	if (*s != '\r' || s[1] != '\n') return -1; // newline always needed

	if (process_http_header_value(header_field, value, req) == -1) return -1;

	s += 2; // \r\n
	*str = s;
	return 0;
}

int process_http_headers(const char** str, HTTPRequest* req) {

	while (true) {
		if (process_http_header(str, req) == -1) return -1;
		if (**str == '\r' && (*str)[1] == '\n') return 0; // empty newline at end of headers
	}

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

static bool is_valid_request_target_relative(char c) {
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
	if (method == HTTP_CONNECT) {
		// rt has to be domain:port format
		char domain[MAX_DOMAIN_LEN + 1] = { 0 };
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		bool with_port = false;
		char value[MAX_DOMAIN_LEN + 1 + MAX_PORT_NUM_CHAR_LEN + 1];
		const char* s = *str;
		int i = 0;
		int len = MAX_DOMAIN_LEN + 1 + MAX_PORT_NUM_CHAR_LEN;
		for (char c = *s; c != '\0' && c != ' ' && i < len; c = *(++s)) {
			value[i] = c;
			i++;
		}
		value[i] = '\0';
		*str = s;

		int res = http_domain_port(value, domain, port, &with_port);
		if (res == -1 || !with_port) return -1;

		strncpy(req->start_line->request_target, value, len);
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

HTTPMethod process_http_method(const char **str, const LookupEntryStrInt *table, const int table_len) {
	
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
	return lookup_str_int(method, table, table_len, false);
}

HTTPRequest* http_request_init() {
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
	HTTPBody* rq_body;
	rq_body = (HTTPBody*)safe_calloc(1, sizeof(*rq_body));
	rq_body->body = body;

	// all

	HTTPRequest *req;
	req = (HTTPRequest*) safe_calloc(1, sizeof(*req));
	req->start_line = sl;
	req->headers = headers;
	req->body = rq_body;
	return req;
}

void http_request_free(HTTPRequest* req) {
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

void http_request_clear(HTTPRequest** req) {
	http_request_free(*req);
	*req = http_request_init();
}

// Response

HTTPResponse* http_response_construct(
	HTTPStatusCode code,
	const char* server_name,
	MediaType content_type,
	const char* body
) {
	HTTPResponse* res = http_response_init();
	res->start_line->status_code = code;
	const char* code_str = lookup_int_str(code, http_status_code_lookup_table, HTTP_STATUS_CODE_TABLE_COUNT);
	if (code_str == NULL) {
		printf("server: response construction failed, reason phrase lookup\n");
		return NULL;
	}
	strncpy(res->start_line->reason_phrase, code_str, MAX_REASON_PHRASE_LEN);

	http_get_current_date(res->headers->date, MAX_DATE_STR_LEN + 1); // + 1 needed, \0 included for strftime
	strncpy(res->headers->server, server_name, MAX_HTTP_HEADER_VALUE_LEN);

	if (body != NULL) {
		size_t body_len = strlen(body);
		if (body_len > MAX_HTTP_BODY_LEN) {
			printf("server: response construction failed, body too long\n");
			return NULL;
		}
		const char* ct_str = lookup_int_str(content_type, media_type_lookup_table, MEDIA_TYPE_TABLE_COUNT);
		if (ct_str == NULL) {
			printf("server: response construction failed, content type lookup\n");
			return NULL;
		}
		strncpy(res->headers->content_type, ct_str, MAX_MEDIA_TYPE_LEN);
		res->headers->content_length = body_len;
		strncpy(res->body->body, body, MAX_HTTP_BODY_LEN);
	}
	else {
		res->headers->content_type = NULL;
		res->headers->content_length = 0;
		res->body->body = NULL;
	}

	return res;
}

int http_header_to_str(HTTPResponseHeaderField field, const char* value, char **buf) {
	const char* field_val;
	field_val = lookup_int_str(field, http_res_header_field_lookup_table, HTTP_RES_HEADER_FIELD_TABLE_COUNT);
	if (field_val == NULL) {
		return -1;
	}
	sprintf(*buf, "%s: %s\r\n", field_val, value);
	*buf += strlen(field_val) + strlen(value) + 4; // ": " + "\r\n" = 4
	return 0;
}

char* http_response_to_str(HTTPResponse* res) {
	int data_len = MAX_HTTP_RES_LEN;
	char* data;
	data = (char*)safe_calloc(MAX_HTTP_RES_LEN + 1, sizeof(*data));
	char* data_start = data;

	sprintf(data, "HTTP/1.1 %d %s\r\n", res->start_line->status_code, res->start_line->reason_phrase);
	data += HTTP_PROT_LEN + 1 + 3 + 1 + strlen(res->start_line->reason_phrase) + 2;
	
	if (http_header_to_str(HTTP_RSH_SERVER, res->headers->server, &data) == -1) { 
		free(data);
		return NULL;
	};
	if (http_header_to_str(HTTP_RSH_DATE, res->headers->date, &data) == -1) {
		free(data);
		return NULL;
	};
	if (http_header_to_str(HTTP_RSH_CONTENT_TYPE, res->headers->content_type, &data) == -1) {
		free(data);
		return NULL;
	};
	char cl[20];
	int_to_str(res->headers->content_length, cl);
	if (http_header_to_str(HTTP_RSH_CONTENT_LENGTH, cl, &data) == -1) {
		free(data);
		return NULL;
	};

	if (res->body->body == NULL) return data_start;

	sprintf(data, "\r\n%s", res->body->body);

	return data_start;
}

int http_response_send(SOCKET inc_sock, SOCKET server_sock, HTTPResponse* res, fd_set* main) {
	if (inc_sock == server_sock) {
		printf("server: cannot send HTTP response to itself\n");
		return -1;
	}
	if (!FD_ISSET(inc_sock, main)) { 
		printf("server: socket is not in set\n");
		return -1;
	};

	char* data = http_response_to_str(res);
	if (data == NULL) {
		printf("server: failed to convert HTTP response to str\n");
		return -1;
	}

	int r = socket_send(inc_sock, data, strlen(data), 0);
	if (r == -1) {
		printf("server: couldn't send data to ");
		socket_print(inc_sock);
		printf("\n");
	}
	free(data);
}

HTTPResponse* http_response_init() {
	// start line
	char* reason_phrase;
	reason_phrase = (char*)safe_calloc(MAX_REASON_PHRASE_LEN + 1, sizeof(*reason_phrase));

	HTTPResponseStartLine* sl;
	sl = (HTTPResponseStartLine*)safe_calloc(1, sizeof(*sl));
	sl->reason_phrase = reason_phrase;

	// headers

	char* server, *date, *content_type;
	server = (char*)safe_calloc(MAX_HTTP_HEADER_VALUE_LEN + 1, sizeof(*server));
	date = (char*)safe_calloc(MAX_DATE_STR_LEN + 1, sizeof(*date));
	content_type = (char*)safe_calloc(MAX_MEDIA_TYPE_LEN + 1, sizeof(*content_type));

	HTTPResponseHeaders* headers;
	headers = (HTTPResponseHeaders*)safe_calloc(1, sizeof(*headers));
	headers->server = server;
	headers->content_type = content_type;
	headers->date = date;

	// body

	char* body;
	body = (char*)safe_calloc(MAX_HTTP_BODY_LEN + 1, sizeof(char*));
	HTTPBody* rs_body;
	rs_body = (HTTPBody*)safe_calloc(1, sizeof(*rs_body));
	rs_body->body = body;

	// all

	HTTPResponse* req;
	req = (HTTPResponse*)safe_calloc(1, sizeof(*req));
	req->start_line = sl;
	req->headers = headers;
	req->body = rs_body;
	return req;
}

void http_response_free(HTTPResponse* res) {
	// start line

	free(res->start_line->reason_phrase);
	free(res->start_line);

	// headers

	free(res->headers->server);
	free(res->headers->date);
	free(res->headers->content_type);
	free(res->headers);

	// body

	free(res->body->body);
	free(res->body);

	// all

	free(res);
}

void http_response_clear(HTTPResponse** res) {
	http_response_free(*res);
	*res = http_response_init();
}

void http_get_current_date(char *str, size_t str_len) {
	struct tm* timeinfo;
	get_current_time_gmt(&timeinfo);

	strftime(str, str_len, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
}
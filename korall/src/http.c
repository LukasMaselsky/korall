#include "http_internal.h"
#include "lookup_tables.h"
#include "http_headers.h"
#include "sockets.h"

/*
	Check if the format of the HTTP request is correct
*/
HTTPError http_parse_request(const char* data, int data_len, HTTPRequest* req) {
	
	HTTPError res = http_process_method(&data, req);
	if (res != HTTP_SUCCESS) return res;
	
	if (*data != ' ') return -1;
	data++; // advance past space

	// process rt
	res = http_process_request_target(&data, req);
	if (res != HTTP_SUCCESS) return res;

	if (*data != ' ') return -1;
	data++; // advance past space

	// process prot
	res = http_process_protocol(&data);
	if (res != HTTP_SUCCESS) return res;

	if (*data != '\r' || data[1] != '\n') return HTTP_BAD_REQUEST; // must have \r\n
	data += 2;

	if (*data == '\r' && data[1] == '\n' && data[2] == '\0') return HTTP_SUCCESS; // no header, no body

	res = http_process_headers(&data, req);
	if (res != HTTP_SUCCESS) return res;

	if (data[0] == '\0') return HTTP_SUCCESS; // no body

	http_process_body(data, req);

	return HTTP_SUCCESS;
}

HTTPError http_process_header_value(const HTTPRequestHeaderField field, const char* value, HTTPRequest *req) {
	// massive switch for each header
	switch (field) {
		case HTTP_RQH_HOST:
			return http_process_host(value, req);
		case HTTP_RQH_ACCEPT:
			return http_process_accept(value, req);
		case HTTP_RQH_ACCEPT_ENCODING:
			return http_process_accept_encoding(value, req);
		case HTTP_RQH_CONTENT_LENGTH:
			return http_process_content_length(value, req);
		case HTTP_RQH_CONTENT_TYPE:
			return http_process_content_type(value, req);
		case HTTP_RQH_ACCESS_CONTROL_REQUEST_METHOD:
			return http_process_access_control_request_method(value, req);
		case HTTP_RQH_ACCESS_CONTROL_REQUEST_HEADERS:
			return http_process_access_control_request_headers(value, req);
		case HTTP_RQH_CONNECTION:
			return http_process_connection(value, req);
		case HTTP_RQH_CACHE_CONTROL:
			return http_process_cache_control(value, req);
		case HTTP_RQH_USER_AGENT:
			return http_process_user_agent(value, req);
		case HTTP_RQH_DATE:
			return http_process_date(value, req);
		case HTTP_RQH_EXPECT:
			return http_process_expect(value, req);
		default:
			return HTTP_BAD_HEADER_VAL; // todo: allow custom headers
	}
}

HTTPError http_process_header(const char** str, HTTPRequest* req) {
	// process field

	char field[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 };
	
	int res = fill_string_char(str, field, MAX_HTTP_HEADER_FIELD_LEN, ':');
	if (res == -1) return HTTP_BAD_HEADER;
	const char *s = *str;

	int header_field = lookup_str_int(field, &http_req_header_field_lookup_table, true);
	if (header_field == -1) return HTTP_BAD_HEADER;

	
	s++; // skip colon
	// process value

	// skip through whitespace
	while (*s == ' ')
		s++;

	if (*s == '\0' || *s == '\n') return HTTP_BAD_HEADER_VAL; // empty field e.g. "Host:    ";

	char value[MAX_HTTP_HEADER_VALUE_LEN + 1] = { 0 };
	*str = s;
	res = fill_string_str(str, value, MAX_HTTP_HEADER_VALUE_LEN, "\r\n");
	if (res == -1) return HTTP_BAD_HEADER_VAL;
	s = *str;

	HTTPError hv_res = http_process_header_value(header_field, value, req);
	if (hv_res != HTTP_SUCCESS) return hv_res;

	s += 2; // \r\n
	*str = s;
	return HTTP_SUCCESS;
}

HTTPError http_process_headers(const char** str, HTTPRequest* req) {

	while (true) {
		HTTPError res = http_process_header(str, req);
		if (res != HTTP_SUCCESS) return res;
		if ((*str)[0] == '\r' && (*str)[1] == '\n') { 
			*str += 2;
			return HTTP_SUCCESS;
		}
		// empty newline at end of headers, required
	}

}

HTTPError http_process_body(const char* str, HTTPRequest* req) {
	// todo

	HTTPMethod method = req->start_line->method;
	if (!(method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_POST)) return HTTP_BODY_NOT_ALLOWED;

	size_t body_len = strlen(str);
	if (body_len > MAX_HTTP_BODY_LEN) return HTTP_BODY_TOO_LONG;
	
	memcpy(req->body, str, body_len);
	req->body[body_len] = '\0';
	
	return HTTP_SUCCESS;
}

HTTPError http_process_protocol(const char** str) {
	char prot[HTTP_PROT_LEN + 1] = { 0 };

	int res = fill_string_str(str, prot, HTTP_PROT_LEN, "\r\n");
	if (res == -1) return HTTP_BAD_PROT;

	if (strncmp("HTTP/1.1", prot, HTTP_PROT_LEN) == 0) return HTTP_SUCCESS;
	return HTTP_BAD_PROT;
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

HTTPError http_process_request_target_relative(const char** str, HTTPRequest* req) {
	// todo: query string
	int len = MAX_HTTP_URL_LEN;
	int i = 0;
	char* rt = req->start_line->request_target;
	const char* s = *str;
	for (char c = *s; c != '\0' && c != ' ' && i < len - 1; c = *(++s)) {
		if (!is_valid_request_target_relative(c)) return HTTP_BAD_REQUEST_TARGET;
		if (i != 0) {
			if (c == '/' && *(s - 1) == '/') return HTTP_BAD_REQUEST_TARGET;
		}
		rt[i] = c;
		i++;
	}
	rt[i] = '\0';
	*str = s;

	return HTTP_SUCCESS;
}

HTTPError http_process_request_target(const char **str, HTTPRequest *req) {
	HTTPMethod method = req->start_line->method;
	if (method == -1) return HTTP_ERROR; // for testing purposes, never actually -1
	const char* s = *str;
	if (method == HTTP_OPTIONS && s[0] == '*') {
		(req->start_line->request_target)[0] = '*';
		(req->start_line->request_target)[1] = '\0';
		*str = ++s;
		return HTTP_SUCCESS;
	}
	if (method == HTTP_CONNECT) {
		// rt has to be domain:port format
		char domain[MAX_DOMAIN_LEN + 1] = { 0 }; // todo: not using these ???
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		bool with_port = false;
		char value[MAX_DOMAIN_LEN + 1 + MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		int len = MAX_DOMAIN_LEN + 1 + MAX_PORT_NUM_CHAR_LEN;
		int res = fill_string_char(str, value, len, ' ');
		if (res == -1) return HTTP_BAD_REQUEST_TARGET;

		HTTPError dp_res = http_domain_port(value, domain, port, &with_port);
		if (dp_res != HTTP_SUCCESS) return dp_res;
		if (!with_port) return HTTP_BAD_PORT;

		strncpy(req->start_line->request_target, value, len);
		return HTTP_SUCCESS;
	}

	const char first_c = s[0];
	if (first_c == '/') {
		// relative path
		return http_process_request_target_relative(str, req);
	}
	else if (first_c == 'h' || first_c == 'H') { // http...
		// absolute path
		// todo
		printf("Absolute paths not supported");
		return HTTP_BAD_REQUEST_TARGET;
	}

	return HTTP_BAD_REQUEST_TARGET;
}

HTTPError http_process_method(const char **str, HTTPRequest *req) {

	char method[MAX_HTTP_METHOD_STR_LEN + 1] = { 0 };
	int len = MAX_HTTP_METHOD_STR_LEN;

	int res = fill_string_char(str, method, len, ' ');
	if (res == -1) return HTTP_BAD_METHOD;

	int method_int = lookup_str_int(method, &http_method_lookup_table, false);
	if (method_int == -1) return HTTP_BAD_METHOD;

	req->start_line->method = method_int;
	return HTTP_SUCCESS;
}

HTTPRequest* http_request_init(Arena *arena) {
	// start line
	char* request_target;
	request_target = (char*) arena_alloc(arena, (MAX_HTTP_URL_LEN + 1) * sizeof(*request_target));

	HTTPRequestStartLine* sl;
	sl = (HTTPRequestStartLine*)arena_alloc(arena, 1 * sizeof(*sl));
	sl->request_target = request_target;
	sl->method = HTTP_METHOD_UNUSED;

	// headers

	HTTPRequestHeaders* headers = http_request_init_headers(arena);

	// body

	char* body;
	body = (char*)arena_alloc(arena, (MAX_HTTP_BODY_LEN + 1) * sizeof(*body));

	// all

	HTTPRequest *req;
	req = (HTTPRequest*)arena_alloc(arena, 1 * sizeof(*req));
	req->start_line = sl;
	req->headers = headers;
	req->body = body;
	return req;
}

HTTPRequestHeaders* http_request_init_headers(Arena* arena) {

	char* domain, * port;
	domain = (char*)arena_alloc(arena, (MAX_DOMAIN_LEN + 1) * sizeof(*domain));
	port = (char*)arena_alloc(arena, (MAX_PORT_NUM_CHAR_LEN + 1) * sizeof(*port));

	HTTPHeaderHost* hh;
	hh = (HTTPHeaderHost*)arena_alloc(arena, 1 * sizeof(*hh));
	hh->domain = domain;
	hh->port = port;

	HTTPWeightedField* mtw_data;
	mtw_data = (HTTPWeightedField*)arena_alloc(arena, HTTP_MEDIA_TYPE_TABLE_COUNT * sizeof(*mtw_data));
	Array* mtw;
	mtw = (Array*)arena_alloc(arena, sizeof(*mtw)); // todo: change count ?
	array_init(mtw, mtw_data, sizeof(*mtw_data), HTTP_MEDIA_TYPE_TABLE_COUNT);

	HTTPWeightedField* ew_data;
	ew_data = (HTTPWeightedField*)arena_alloc(arena, HTTP_ENCODING_TABLE_COUNT * sizeof(*ew_data));
	Array* ew;
	ew = (Array*)arena_alloc(arena, sizeof(*ew)); // todo: change count ?
	array_init(ew, ew_data, sizeof(*ew_data), HTTP_ENCODING_TABLE_COUNT);


	char* boundary;
	boundary = (char*)arena_alloc(arena, (MAX_HTTP_BOUNDARY_LEN + 1) * sizeof(*boundary));

	HTTPContentType* ct;
	ct = (HTTPContentType*)arena_alloc(arena, 1 * sizeof(*ct));
	ct->boundary = boundary;
	ct->charset = HTTP_CHS_UNUSED;
	ct->media_type = HTTP_MT_UNUSED;

	HTTPRequestHeaderField* acrh_data;
	acrh_data = (HTTPRequestHeaderField*)arena_alloc(arena, HTTP_RQH_COUNT * sizeof(*acrh_data));
	Array* acrh;
	acrh = (Array*)arena_alloc(arena, sizeof(*acrh));
	array_init(acrh, acrh_data, sizeof(*acrh_data), HTTP_RQH_COUNT);

	HTTPRequestCacheControlPair* cc_data;
	cc_data = (HTTPRequestCacheControlPair*)arena_alloc(arena, HTTP_REQ_CC_COUNT * sizeof(*cc_data));
	Array* cc;
	cc = (Array*)arena_alloc(arena, sizeof(*cc));
	array_init(cc, cc_data, sizeof(*cc_data), HTTP_REQ_CC_COUNT);

	char* user_agent;
	user_agent = (char*)arena_alloc(arena, (MAX_HTTP_USER_AGENT + 1) * sizeof(*user_agent));

	HTTPDate* date;
	date = (HTTPDate*)arena_alloc(arena, 1 * sizeof(*date));
	date->used = false;

	HTTPRequestHeaders* headers;
	headers = (HTTPRequestHeaders*)arena_alloc(arena, 1 * sizeof(*headers));
	headers->host = hh;
	headers->accept = mtw;
	headers->accept_encoding = ew;
	headers->content_type = ct;
	headers->content_length = -1;
	headers->access_control_request_method = HTTP_METHOD_UNUSED;
	headers->access_control_request_headers = acrh;
	headers->connection = HTTP_CON_UNUSED;
	headers->cache_control = cc;
	headers->user_agent = user_agent;
	headers->date = date;
	headers->expect = HTTP_EXP_UNUSED;
	return headers;
}

void http_request_free(Arena *arena, HTTPRequest* req) {
	arena_free(arena);
}

void http_request_clear(Arena *arena, HTTPRequest** req) {
	arena_clear(arena);
	*req = http_request_init(arena);
}

// Response

HTTPResponse* http_response_construct(
	Arena* arena,
	HTTPStatusCode code,
	const char* server_name,
	HTTPMediaType content_type,
	const char* body
) {
	HTTPResponse* res = http_response_init(arena);
	res->start_line->status_code = code;
	const char* code_str = lookup_int_str(code, &http_status_code_lookup_table);
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
		const char* ct_str = lookup_int_str(content_type, &http_media_type_lookup_table);
		if (ct_str == NULL) {
			printf("server: response construction failed, content type lookup\n");
			return NULL;
		}
		strncpy(res->headers->content_type, ct_str, MAX_MEDIA_TYPE_LEN);
		res->headers->content_length = body_len;
		strncpy(res->body, body, MAX_HTTP_BODY_LEN);
	}
	else {
		res->headers->content_type = NULL;
		res->headers->content_length = 0;
		res->body = NULL;
	}

	return res;
}

int http_header_to_str(HTTPResponseHeaderField field, const char* value, char **buf) {
	const char* field_val;
	field_val = lookup_int_str(field, &http_res_header_field_lookup_table);
	if (field_val == NULL) {
		return -1;
	}
	sprintf(*buf, "%s: %s\r\n", field_val, value);
	*buf += strlen(field_val) + strlen(value) + 4; // ": " + "\r\n" = 4
	return 0;
}

char* http_response_to_str(const HTTPResponse* res) {
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

	if (res->body == NULL) return data_start;

	sprintf(data, "\r\n%s", res->body);

	return data_start;
}

int http_response_send(const SOCKET inc_sock, const SOCKET server_sock, const HTTPResponse* res, const fd_set* main) {
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
	return 0;
}

HTTPResponse* http_response_init(Arena *arena) {
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

	// all

	HTTPResponse* req;
	req = (HTTPResponse*)safe_calloc(1, sizeof(*req));
	req->start_line = sl;
	req->headers = headers;
	req->body = body;
	return req;
}

void http_response_free(Arena* arena, HTTPResponse* res) {
	arena_free(arena);
}

void http_response_clear(Arena* arena, HTTPResponse** res) {
	arena_clear(arena);
	*res = http_response_init(arena);
}

void http_get_current_date(char *str, size_t str_len) {
	struct tm* timeinfo;
	get_current_time_gmt(&timeinfo);

	strftime(str, str_len, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
}

/* 
	Get response info based on HTTPError
*/
const char* http_error_response_info(HTTPError error_code, HTTPStatusCode* sc, HTTPMediaType* mt) {
	*sc = HTTP_SC_400;
	*mt = HTTP_MT_APP_JSON;

	switch (error_code) {
		case HTTP_BODY_TOO_LONG:
			return ERROR_MESSAGE("Bad request", "Maximum body length is 1MB.");
		case HTTP_BODY_NOT_ALLOWED:
			return ERROR_MESSAGE("Bad request", "Body is only allowed for PUT, PATCH and POST requests.");
		case HTTP_BAD_EXPECT:
			return ERROR_MESSAGE("Bad request", "Invalid Expect header.");
		case HTTP_BAD_DATE:
			return ERROR_MESSAGE("Bad request", "Invalid Date header.");
		case HTTP_BAD_USER_AGENT:
			*sc = HTTP_SC_413;
			return ERROR_MESSAGE("Bad request", "Maximum User-Agent field value is 4KB.");
		case HTTP_BAD_CACHE_CONTROL:
			return ERROR_MESSAGE("Bad request", "Invalid Cache-Control header.");
		case HTTP_BAD_CONNECTION:
			return ERROR_MESSAGE("Bad request", "Invalid Connection header.");
		case HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS:
			return ERROR_MESSAGE("Bad request", "Invalid Access-Control-Request-Headers header.");
		case HTTP_BAD_ACCESS_CONTROL_REQUEST_METHOD:
			return ERROR_MESSAGE("Bad request", "Invalid Access-Control-Request-Method header.");
		case HTTP_BAD_CONTENT_TYPE:
			return ERROR_MESSAGE("Bad request", "Invalid Content-Type header.");
		case HTTP_BAD_CONTENT_LENGTH:
			return ERROR_MESSAGE("Bad request", "Invalid Content-Length header.");
		case HTTP_BAD_ACCEPT_ENC:
			return ERROR_MESSAGE("Bad request", "Invalid Accept-Encoding header.");
		case HTTP_BAD_ACCEPT:
			return ERROR_MESSAGE("Bad request", "Invalid Accept header.");
		case HTTP_BAD_PROT:
			return ERROR_MESSAGE("Bad request", "Invalid HTTP protocol.");
		case HTTP_BAD_HEADER_VAL:
			return ERROR_MESSAGE("Bad request", "Unsupported header.");
		case HTTP_BAD_HEADER:
			return ERROR_MESSAGE("Bad request", "Invalid header.");
		case HTTP_BAD_PORT:
			return ERROR_MESSAGE("Bad request", "Invalid port.");
		case HTTP_BAD_DOMAIN:
			return ERROR_MESSAGE("Bad request", "Invalid domain.");
		case HTTP_BAD_DOMAIN_PORT:
			return ERROR_MESSAGE("Bad request", "Invalid domain or port.");
		case HTTP_BAD_REQUEST:
			return ERROR_MESSAGE("Bad request", "Invalid request format.");
		case HTTP_BAD_REQUEST_TARGET:
			return ERROR_MESSAGE("Bad request", "Invalid request target.");
		case HTTP_BAD_METHOD:
			return ERROR_MESSAGE("Bad request", "Invalid method.");
		case HTTP_ERROR:
		default:
			return ERROR_MESSAGE("Bad request", "Invalid request.");
	}
}
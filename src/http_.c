#include "http_.h"
#include "utils.h"
#include "lookup_tables.h"
#include "sockets.h"
#include "http_headers.h"
#include "arena.h"
#include "array.h"

/*
	Check if the format of the HTTP request is correct
*/
int http_validate_request(const char* data, int data_len, HTTPRequest* req) {
	const LookupEntry* table = &http_method_lookup_table;
	const int table_len = HTTP_METHOD_LOOKUP_TABLE_COUNT;

	HTTPMethod method = http_process_method(&data, table, table_len);
	if (method == HTTP_BADMETHOD || data[0] == '\0') return -1;
	req->start_line->method = method;
	if (*data != ' ') return -1;
	data++; // advance past space

	// process rt
	int res = http_process_request_target(&data, req);
	if (res == -1) return -1;
	if (*data != ' ') return -1;
	data++; // advance past space

	// process prot
	res = http_process_protocol(&data);
	if (res == -1) return -1;

	if (*data != '\r' || data[1] != '\n') return -1; // must have \r\n
	data += 2;

	if (*data == '\r' && data[1] == '\n' && data[2] == '\0') return 0; // no header, no body

	res = http_process_headers(&data, req);
	if (res == -1) return -1;

	printf(data);
	// todo: body
	// Only patch, put and post has body

	return 0;
}

int http_process_header_value(const HTTPRequestHeaderField field, const char* value, HTTPRequest *req) {
	// massive switch for each header
	switch (field) {
		case HTTP_RQH_HOST:
			return http_process_host(value, req);
		case HTTP_RQH_ACCEPT:
			return http_process_accept(value, req);
		case HTTP_RQH_CONTENT_LENGTH:
			return http_process_content_length(value, req);
		case HTTP_RQH_CONTENT_TYPE:
			return http_process_content_type(value, req);
		default:
			return -1; // todo: allow custom headers
	}
}

int http_process_header(const char** str, HTTPRequest* req) {
	// process field

	char field[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 };
	
	int res = fill_string_char(str, field, MAX_HTTP_HEADER_FIELD_LEN, ':');
	if (res == -1) return -1;
	const char *s = *str;

	HTTPRequestHeaderField header_field = lookup_str_int(field, http_req_header_field_lookup_table, HTTP_REQ_HEADER_FIELD_TABLE_COUNT, true);
	if (header_field == HTTP_RQH_BADFIELD) return -1;

	
	s++; // skip colon
	// process value

	// skip through whitespace
	while (*s == ' ')
		s++;

	if (*s == '\0' || *s == '\n') return -1; // empty field e.g. "Host:    ";

	char value[MAX_HTTP_HEADER_VALUE_LEN + 1] = { 0 };
	*str = s;
	res = fill_string_str(str, value, MAX_HTTP_HEADER_VALUE_LEN, "\r\n");
	if (res == -1) return -1;
	s = *str;

	if (http_process_header_value(header_field, value, req) == -1) return -1;

	s += 2; // \r\n
	*str = s;
	return 0;
}

int http_process_headers(const char** str, HTTPRequest* req) {

	while (true) {
		if (http_process_header(str, req) == -1) return -1;
		if ((*str)[0] == '\r' && (*str)[1] == '\n') return 0; // empty newline at end of headers, required
	}

}

int http_process_protocol(const char** str) {
	char prot[HTTP_PROT_LEN + 1] = { 0 };

	int res = fill_string_str(str, prot, HTTP_PROT_LEN, "\r\n");
	if (res == -1) return -1;

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

int http_process_request_target_relative(const char** str, HTTPRequest* req) {
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

int http_process_request_target(const char **str, HTTPRequest *req) {
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
		char domain[MAX_DOMAIN_LEN + 1] = { 0 }; // todo: not using these ???
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		bool with_port = false;
		char value[MAX_DOMAIN_LEN + 1 + MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		int len = MAX_DOMAIN_LEN + 1 + MAX_PORT_NUM_CHAR_LEN;
		int res = fill_string_char(str, value, len, ' ');
		if (res == -1) return -1;

		res = http_domain_port(value, domain, port, &with_port);
		if (res == -1 || !with_port) return -1;

		strncpy(req->start_line->request_target, value, len);
		return 0;
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
		return -1;
	}
	else {
		return -1;
	}

	return -1;
}

HTTPMethod http_process_method(const char **str, const LookupEntry *table, const int table_len) {
	
	char method[MAX_HTTP_METHOD_STR_LEN + 1] = { 0 };
	int len = MAX_HTTP_METHOD_STR_LEN;

	int res = fill_string_char(str, method, len, ' ');
	if (res == -1) return -1;

	return lookup_str_int(method, table, table_len, false);
}

HTTPRequest* http_request_init(Arena *arena) {
	// start line
	char* request_target;
	request_target = (char*) arena_alloc(arena, (MAX_HTTP_URL_LEN + 1) * sizeof(*request_target));

	HTTPRequestStartLine* sl;
	sl = (HTTPRequestStartLine*)arena_alloc(arena, 1 * sizeof(*sl));
	sl->request_target = request_target;

	// headers

	char *domain, *port;
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


	HTTPRequestHeaders* headers;
	headers = (HTTPRequestHeaders*)arena_alloc(arena, 1 * sizeof(*headers));
	headers->host = hh;
	headers->accept = mtw;
	headers->accept_encoding = ew;
	headers->content_type = ct;

	// body

	char* body;
	body = (char*)arena_alloc(arena, (MAX_HTTP_BODY_LEN + 1) * sizeof(*body));
	HTTPBody* rq_body;
	rq_body = (HTTPBody*)arena_alloc(arena, 1 * sizeof(*rq_body));
	rq_body->body = body;

	// all

	HTTPRequest *req;
	req = (HTTPRequest*)arena_alloc(arena, 1 * sizeof(*req));
	req->start_line = sl;
	req->headers = headers;
	req->body = rq_body;
	return req;
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
		const char* ct_str = lookup_int_str(content_type, http_media_type_lookup_table, HTTP_MEDIA_TYPE_TABLE_COUNT);
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
#include "http/http_internal.h"
#include "lookup/lookup_tables.h"
#include "http/http_headers.h"
#include "socket/socket.h"
#include "korall_internal.h"

// Request

/*
	Check if the format of the HTTP request is correct
*/
HTTPError http_request_parse(const char* data, HTTPRequest* req) {
	
	HTTPError res = http_request_process_method(&data, req);
	if (res != HTTP_SUCCESS) return res;
	
	if (*data != ' ') return -1;
	data++; // advance past space

	// process rt
	res = http_request_process_target(&data, req);
	if (res != HTTP_SUCCESS) return res;

	if (*data != ' ') return -1;
	data++; // advance past space

	// process prot
	res = http_request_process_protocol(&data);
	if (res != HTTP_SUCCESS) return res;

	if (*data != '\r' || data[1] != '\n') return HTTP_BAD_REQUEST; // must have \r\n
	data += 2;

	if (*data == '\r' && data[1] == '\n' && data[2] == '\0') return HTTP_SUCCESS; // no header, no body

	res = http_request_process_headers(&data, req);
	if (res != HTTP_SUCCESS) return res;

	if (data[0] == '\0') return HTTP_SUCCESS; // no body

	http_request_process_body(data, req);

	return HTTP_SUCCESS;
}

HTTPError http_process_request_header_value(const HTTPRequestHeaderField field, const char* value, HTTPRequest *req) {
	// massive switch for each header
	switch (field) {
		case HTTP_RQH_ACCEPT_LANGUAGE:
			return http_process_accept_language(value);
		case HTTP_RQH_CONTENT_ENCODING:
			return http_process_content_encoding(value);
		case HTTP_RQH_HOST:
			return http_process_host(value, req);
		case HTTP_RQH_ACCEPT:
			return http_process_accept(value);
		case HTTP_RQH_ACCEPT_ENCODING:
			return http_process_accept_encoding(value);
		case HTTP_RQH_CONTENT_LENGTH:
			return http_process_content_length(value);
		case HTTP_RQH_CONTENT_TYPE:
			return http_process_content_type(value);
		case HTTP_RQH_ACCESS_CONTROL_REQUEST_METHOD:
			return http_process_access_control_request_method(value);
		case HTTP_RQH_ACCESS_CONTROL_REQUEST_HEADERS:
			return http_process_access_control_request_headers(value);
		case HTTP_RQH_CONNECTION:
			return http_process_connection(value, req);
		case HTTP_RQH_CACHE_CONTROL:
			return http_process_cache_control_req(value);
		case HTTP_RQH_USER_AGENT:
			return http_process_user_agent(value);
		case HTTP_RQH_DATE:
			return http_process_date(value);
		case HTTP_RQH_EXPECT:
			return http_process_expect(value);
		case HTTP_RQH_TE:
			return http_process_te(value);
		case HTTP_RQH_TRANSFER_ENCODING:
			return http_process_transfer_encoding(value);
		case HTTP_RQH_MAX_FORWARDS:
			return http_process_max_forwards(value);
		case HTTP_RQH_UPGRADE:
			return http_process_upgrade(value, req);
		case HTTP_RQH_WS_KEY:
			return http_process_ws_key(value, req);
		case HTTP_RQH_WS_VERSION:
			return http_process_ws_version(value, req);
		default:
			return g_config.allow_custom_headers ? HTTP_SUCCESS : HTTP_BAD_HEADER_VAL;
	}
}

HTTPError http_request_process_header(const char** str, HTTPRequest* req) {
	// process field
	// todo: full header line instead of field and value len separate

	char field[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 }; // todo: allow custom headers (longer header field?)
	
	int res = fill_string_char(str, field, MAX_HTTP_HEADER_FIELD_LEN, ':');
	if (res == -1) return HTTP_BAD_HEADER;
	const char *s = *str;

	int header_field = lookup_str_int(field, &http_req_header_field_lookup_table, true);
	if (header_field == -1 && !(g_config.allow_custom_headers)) return HTTP_BAD_HEADER; // todo: better error msg

	
	s++; // skip colon
	// process value

	// skip through whitespace
	while (*s == ' ')
		s++;

	if (*s == '\0' || *s == '\n') return HTTP_BAD_HEADER_VAL; // empty field e.g. "Host:    ";

	char value[MAX_HTTP_HEADER_VALUE_LEN + 1] = { 0 };
	*str = s;
	res = fill_string_str(str, value, MAX_HTTP_HEADER_VALUE_LEN, "\r\n", false);
	if (res == -1) return HTTP_BAD_HEADER_VAL;

	HTTPError hv_res = http_process_request_header_value(header_field, value, req);
	if (hv_res != HTTP_SUCCESS) return hv_res;

	*str += 2; // \r\n
	return HTTP_SUCCESS;
}

HTTPError http_request_process_headers(const char** str, HTTPRequest* req) {

	const char* base = *str;

	while (true) {
		HTTPError res = http_request_process_header(str, req);
		if (res != HTTP_SUCCESS) return res;
		if ((*str)[0] == '\r' && (*str)[1] == '\n') { 
			*str += 2;
			// copy whole headers
			if (fill_string_str(&base, req->headers, HTTP_REQ_HEADERS_LEN, "\r\n\r\n", false) == -1) return HTTP_ERROR;
			return HTTP_SUCCESS;
		}
		// empty newline at end of headers, required
	}

}

HTTPError http_request_process_body(const char* str, HTTPRequest* req) {
	HTTPMethod method = req->start_line->method;
	if (!(method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_POST)) return HTTP_BODY_NOT_ALLOWED;

	size_t body_len = strlen(str);
	if (body_len > HTTP_REQ_BODY_LEN) return HTTP_BODY_TOO_LONG;
	
	memcpy(req->body, str, body_len);
	req->body[body_len] = '\0';
	
	return HTTP_SUCCESS;
}

HTTPError http_request_process_protocol(const char** str) {
	char prot[HTTP_PROT_LEN + 1] = { 0 };

	int res = fill_string_str(str, prot, HTTP_PROT_LEN, "\r\n", false);
	if (res == -1) return HTTP_BAD_PROT;

	if (strncmp("HTTP/1.1", prot, HTTP_PROT_LEN) == 0) return HTTP_SUCCESS;
	return HTTP_BAD_PROT;
}

static bool is_valid_query_string(const char c) {
	if (c <= 'z' && c >= 'a') return true;
	if (c <= 'Z' && c >= 'A') return true;
	if (c <= '9' && c >= '0') return true;
	if (c <= '.' && c >= '\'') return true;
	if (c == '$' || c == '!' || c == '_' || c == '~' || c == '%') return true;
	return false;
}

static bool is_valid_path_segment(const char c) {
	if (c <= 'z' && c >= 'a') return true;
	if (c <= 'Z' && c >= 'A') return true;
	if (c <= '9' && c >= '0') return true;
	if (c <= '.' && c >= '$') return true;
	if (c == '!' || c == ':' || c == ';' || c == '=' || c == '_' || c == '~' || c == '@' || c == '%') return true;

	return false;
}

static bool is_valid_hostname_label(const char c) {
	if (c <= 'z' && c >= 'a') return true;
	if (c <= 'Z' && c >= 'A') return true;
	if (c <= '9' && c >= '0') return true;
	if (c == '-') return true;
	return false;
}

HTTPError http_request_process_target_relative(const char* str, HTTPRequest* req) {
	if (str[0] != '/') return HTTP_BAD_REQUEST_TARGET;
	const char* base = str;
	str++;
	
	int seg_len = 0;
	for (char c = *str; c != '\0'; c = *(++str)) {
		if (c == '/') {
			if (seg_len < 1) return HTTP_BAD_REQUEST_TARGET;
			seg_len = 0;
			continue;
		}
		if (c == '%') {
			// safe since null term is false
			if (!(is_hex_digit(str[1]) && is_hex_digit(str[2]))) return HTTP_BAD_REQUEST_TARGET;
			continue;
		}
		if (c == '?') break;
		if (!is_valid_path_segment(c)) return HTTP_BAD_REQUEST_TARGET;
		seg_len++;
	}

	if (*str == '?') {
		// query params
		str++;
		if (*str == '\0') return HTTP_BAD_REQUEST_TARGET; // cannot have ".../?" ?
		int part_len = 0;
		bool in_val = false;
		bool in_field = true;
		for (char c = *str; c != '\0'; c = *(++str)) {
			if (c == '=') {
				if (part_len < 1 || !in_field) return HTTP_BAD_REQUEST_TARGET;
				part_len = 0;
				in_val = true;
				in_field = false;
				continue;
			}
			if (c == '&') {
				if (part_len < 1 || !in_val) return HTTP_BAD_REQUEST_TARGET;
				part_len = 0;
				in_val = false;
				in_field = true;
				continue;
			}
			if (c == '%') {
				// safe since null term is false
				if (!(is_hex_digit(str[1]) && is_hex_digit(str[2]))) return HTTP_BAD_REQUEST_TARGET;
				continue;
			}

			if (!is_valid_path_segment(c)) return HTTP_BAD_REQUEST_TARGET;
			part_len++;
		}
		if (!in_val) return HTTP_BAD_REQUEST_TARGET; // must end on value
	}


	strncpy(req->start_line->request_target, base, MAX_HTTP_URL_LEN);
	return HTTP_SUCCESS;
}

HTTPError http_request_process_target_absolute(const char* str, HTTPRequest* req) {
	const char* base = str;
	char temp[MAX_HTTP_URL_LEN + 1] = { 0 };
	int skip;
	if (fill_string_str(&str, temp, MAX_HTTP_URL_LEN, "http://", true) == 0) {
		skip = 7;
	}
	else if (fill_string_str(&str, temp, MAX_HTTP_URL_LEN, "https://", true) == 0) {
		skip = 8;
	}
	else {
		return HTTP_BAD_REQUEST_TARGET;
	}
	if (temp[0] != '\0') return HTTP_BAD_REQUEST_TARGET;
	memset(temp, 0, MAX_HTTP_URL_LEN);
	str += skip;

	// www.
	if (fill_string_str(&str, temp, MAX_HTTP_URL_LEN, "www.", true) == 0) {
		skip = 4;
		if (temp[0] != '\0') return HTTP_BAD_REQUEST_TARGET;
		memset(temp, 0, MAX_HTTP_URL_LEN);
		str += skip;
	}

	// process host name labels
	
	int total_label_len = 0;
	int label_len = 0;
	char prev_c = 0;
	for (char c = *str; c != '\0' && c != '/'; c = *(++str)) {
		if (c == '.') {
			if (label_len > MAX_DOMAIN_LABEL_LEN || label_len < MIN_DOMAIN_LABEL_LEN) return HTTP_BAD_REQUEST_TARGET;
			if (prev_c == '-') return HTTP_BAD_REQUEST_TARGET;
			label_len = 0;
			prev_c = c;
			continue;
		}
		if (!is_valid_hostname_label(c)) return HTTP_BAD_REQUEST_TARGET;
		if (label_len == 0 && c == '-') return HTTP_BAD_REQUEST_TARGET; // cant start with -
		prev_c = c;
		label_len++;
		total_label_len++;
	}
	if (total_label_len > MAX_DOMAIN_LEN || prev_c == '.' || prev_c == '-') return HTTP_BAD_REQUEST_TARGET;

	if (*str == '/')  {
		int err = http_request_process_target_relative(str, req);
		if (err != HTTP_SUCCESS) return err;
	}

	strncpy(req->start_line->request_target, base, MAX_HTTP_URL_LEN);

	return HTTP_SUCCESS;
}

HTTPError http_request_process_target(const char **str, HTTPRequest *req) {
	HTTPMethod method = req->start_line->method;
	if (method == -1) return HTTP_ERROR; // for testing purposes, never actually -1

	char value[MAX_HTTP_URL_LEN + 1] = { 0 };
	if (fill_string_char(str, value, MAX_HTTP_URL_LEN, ' ') == -1) return HTTP_REQUEST_TARGET_TOO_BIG;


	if (method == HTTP_OPTIONS) {
		if (value[0] == '*' && value[1] == '\0') {
			strncpy(req->start_line->request_target, value, 1);
			return HTTP_SUCCESS;
		}
		return HTTP_BAD_REQUEST_TARGET;
	}

	if (method == HTTP_CONNECT) {
		// rt has to be domain:port format
		char domain[MAX_DOMAIN_LEN + 1] = { 0 };
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		bool with_port = false;

		HTTPError dp_res = http_domain_port(value, domain, port, &with_port);
		if (dp_res != HTTP_SUCCESS) return dp_res;
		if (!with_port) return HTTP_BAD_PORT;

		strncpy(req->start_line->request_target, value, MAX_HTTP_URL_LEN);
		return HTTP_SUCCESS;
	}

	const char first_c = value[0];
	if (first_c == '/') {
		// relative path
		return http_request_process_target_relative(value, req);
	}
	else if (first_c == 'h' || first_c == 'H') { // http...
		// absolute path
		return http_request_process_target_absolute(value, req);
	}

	return HTTP_BAD_REQUEST_TARGET;
}

HTTPError http_request_process_method(const char **str, HTTPRequest *req) {

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
	request_target = (char*) arena_alloc(arena, (HTTP_REQ_START_LINE_LEN + 1) * sizeof(*request_target));

	HTTPRequestStartLine* sl;
	sl = (HTTPRequestStartLine*)arena_alloc(arena, 1 * sizeof(*sl));
	sl->request_target = request_target;
	sl->method = HTTP_METHOD_UNUSED;

	// headers

	char* domain, * port;
	domain = (char*)arena_alloc(arena, (MAX_DOMAIN_LEN + 1) * sizeof(*domain));
	port = (char*)arena_alloc(arena, (MAX_PORT_NUM_CHAR_LEN + 1) * sizeof(*port));

	HTTPHeaderHost* host;
	host = (HTTPHeaderHost*)arena_alloc(arena, 1 * sizeof(*host));
	host->domain = domain;
	host->port = port;

	char* headers;
	headers = (char*)arena_alloc(arena, (HTTP_REQ_HEADERS_LEN + 1) * sizeof(*headers));
	
	// body

	char* body;
	body = (char*)arena_alloc(arena, (HTTP_REQ_BODY_LEN + 1) * sizeof(*body));

	// ws

	char* accept;
	accept = (char*)arena_alloc(arena, (28 + 1) * sizeof(*accept));


	HTTPRequestWebsocket* ws;
	ws = (HTTPRequestWebsocket*)arena_alloc(arena, sizeof(*ws));
	ws->accept = accept;

	// all

	HTTPRequest *req;
	req = (HTTPRequest*)arena_alloc(arena, 1 * sizeof(*req));
	req->start_line = sl;
	req->host = host;
	req->headers = headers;
	req->body = body;
	req->ws = ws;
	return req;
}

void http_request_free(Arena *arena) {
	arena_free(arena);
}

void http_request_clear(Arena *arena, HTTPRequest** req) {
	arena_clear(arena);
	*req = http_request_init(arena);
}

// Response

HTTPError http_process_response_header_value(const HTTPResponseHeaderField field, const char* value) {
	// massive switch for each header
	switch (field) {
		case HTTP_RSH_CONTENT_LENGTH:
			return http_process_content_length(value);
		case HTTP_RSH_CONTENT_TYPE:
			return http_process_content_type(value);
		case HTTP_RSH_CONNECTION:
			return http_process_connection(value, NULL);
		case HTTP_RSH_CACHE_CONTROL:
			return http_process_cache_control_res(value);
		case HTTP_RSH_DATE:
			return http_process_date(value);
		case HTTP_RSH_SERVER:
			return http_process_server(value); // todo:
		case HTTP_RSH_TRANSFER_ENCODING:
			return http_process_transfer_encoding(value);
		case HTTP_RSH_TK:
			return http_process_tk(value);
		case HTTP_RSH_UPGRADE:
			return http_process_upgrade(value, NULL);
		default:
			return g_config.allow_custom_headers ? HTTP_SUCCESS : HTTP_BAD_HEADER_VAL;
	}
}

int http_response_construct(
	HTTPResponse *res,
	HTTPStatusCode code,
	const char* server_name,
	HTTPMediaType content_type,
	const char* body
) {
	if (korall_response_start_set(res, code) == -1) return -1;

	if (body != NULL) {
		const char* ct_str = lookup_int_str(content_type, &http_media_type_lookup_table);
		if (ct_str == NULL) {
			logger(LOG_ERR, "response construction failed, content type lookup\n");
			return -1;
		}

		if (korall_response_header_set(res, "Content-Type", ct_str) == -1) return -1;
		
		korall_response_body_set(res, body);
	}
	else {
		if (korall_response_header_set(res, "Content-Length", "0") == -1) return -1;
	}
	return 0;
}

int http_response_ws_construct(
	HTTPResponse* res,
	const char* accept,
	const char* server_name
) {
	if (korall_response_start_set(res, HTTP_SC_101) == -1) return -1;
	if (korall_response_header_set(res, "Server", server_name) == -1) return -1;
	if (korall_response_header_set(res, "Upgrade", "websocket") == -1) return -1;
	if (korall_response_header_set(res, "Connection", "upgrade") == -1) return -1;
	if (korall_response_header_set(res, "Sec-WebSocket-Accept", accept) == -1) return -1;
	if (korall_response_header_set(res, "Content-Length", "0") == -1) return -1; // ! important
	return 0;
}

int http_response_send(const SOCKET inc_sock, const HTTPResponse *res) {

	Arena res_full_arena = arena_init(HTTP_RES_FULL_ARENA_SIZE + 1); // for concating res parts into full response text
	char* data = (char*)arena_alloc(&res_full_arena, HTTP_RES_FULL_ARENA_SIZE + 1);

	const char* body = res->body.chars[0] == '\0' ? "\r\n" : res->body.chars;

	sprintf(data, "%s%s%s", res->start_line.chars, res->headers_base, body);
	if (data == NULL) {
		logger(LOG_ERR, "failed to convert HTTP response to str\n");
		arena_free(&res_full_arena);
		return -1;
	}
	printf("'%s'\n", data);

	int r = socket_send(inc_sock, data, strlen(data), 0);
	if (r == -1) {
		logger(LOG_ERR, "couldn't send data to ");
		socket_print(inc_sock);
		printf("\n");
	}
	arena_free(&res_full_arena);
	return 0;
}

HTTPResponse* http_response_init(Arena *arena) {

	HTTPResponse* res;
	res = (HTTPResponse*)arena_alloc(arena, 1 * sizeof(*res));

	char* start_line, *headers, *body;
	start_line = (char*)arena_alloc(arena, (HTTP_RES_START_LINE_LEN + 1) * sizeof(*start_line));
	headers = (char*)arena_alloc(arena, (HTTP_RES_HEADERS_LEN + 1) * sizeof(*headers));
	body = (char*)arena_alloc(arena, (HTTP_RES_BODY_LEN + 1) * sizeof(*body));

	res->start_line.chars = start_line;
	res->start_line.size = HTTP_RES_START_LINE_LEN;
	res->headers.chars = headers;
	res->headers.size = HTTP_RES_HEADERS_LEN;
	res->header_count = 0;
	res->header_capacity = HTTP_RES_HEADER_COUNT;
	res->header_size = HTTP_RES_HEADER_LEN;
	res->headers_base = headers;
	res->body.chars = body;
	res->body.size = HTTP_RES_BODY_LEN;
	return res;
}

void http_response_free(Arena* arena) {
	arena_free(arena);
}

void http_response_clear(Arena* arena, HTTPResponse** res) {
	arena_clear(arena);
	*res = http_response_init(arena);
}

void http_get_current_date(String *str) {
	struct tm* timeinfo;
	get_current_time_gmt(&timeinfo);

	strftime(str->chars, str->size, "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
}

/* 
	Get response info based on HTTPError
*/
const char* http_error_response_info(HTTPError error_code, HTTPStatusCode* sc, HTTPMediaType* mt) {
	*sc = HTTP_SC_400;
	*mt = HTTP_MT_APP_JSON;

	switch (error_code) {
		case HTTP_BAD_ACCEPT_LANG:
			return ERROR_MESSAGE("Bad request", "Invalid Accept-Language header.");
		case HTTP_BAD_CONTENT_ENC:
			return ERROR_MESSAGE("Bad request", "Invalid Content-Encoding header.");
		case HTTP_BAD_WS_VERSION:
			return ERROR_MESSAGE("Bad request", "Invalid Sec-WebSocket-Version header.");
		case HTTP_BAD_WS_KEY:
			return ERROR_MESSAGE("Bad request", "Invalid Sec-WebSocket-Key header.");
		case HTTP_BAD_WS_KEY_CALC:
			return ERROR_MESSAGE("Bad request", "Sec-WebSocket-Accept calculation failed.");
		case HTTP_BAD_UPGRADE:
			return ERROR_MESSAGE("Bad request", "Invalid Upgrade header.");
		case HTTP_BAD_TK:
			return ERROR_MESSAGE("Bad request", "Invalid Tk header.");
		case HTTP_BAD_MAX_FORWARDS:
			return ERROR_MESSAGE("Bad request", "Invalid Max-Forwards header.");
		case HTTP_BAD_TRANSFER_ENCODING:
			return ERROR_MESSAGE("Bad request", "Invalid Transfer-Encoding header.");
		case HTTP_BAD_TE:
			return ERROR_MESSAGE("Bad request", "Invalid TE header.");
		case HTTP_BODY_TOO_LONG:
			*sc = HTTP_SC_413;
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
			return ERROR_MESSAGE("Bad request", "Invalid header (allow_custom_headers may be set to false).");
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

// PUBLIC

/*
	Get a request param
*/
int korall_request_param_get(const HTTPRequest* req, const char* field, char *value, size_t value_len) {
	const char* rt = req->start_line->request_target;

	// find start of query params
	if (fill_string_char(&rt, NULL, 0, '?') == -1) return -1;
	// find field
	if (fill_string_str(&rt, NULL, 0, field, false) == -1) return -1;
	// find start of value
	if (fill_string_char(&rt, NULL, 0, '=') == -1) return -1;
	rt++;

	if (fill_string_char(&rt, value, value_len, '&') == -1
		&& fill_string_char(&rt, value, value_len, '\0') == -1) return -1;

	return 0;
}

/*
	Get a request header
*/
int korall_request_header_get(const HTTPRequest* req, const char* field, char* value, size_t value_len) {
	// todo: write tests
	const char* h = req->headers;
	const char* base = h;
	if (fill_string_str(&h, NULL, 0, field, false) == -1) return -1;
	
	// make sure actually field and not reading middle of a value
	if (!(h == base || (*(h - 2) == '\r' && *(h - 1) == '\n'))) return -1;

	if (fill_string_char(&h, NULL, 0, ':') == -1) return -1;
	h += 2; // skip : and space

	return fill_string_str(&h, value, value_len, "\r\n", false);
}

/*
	Get request body
*/
char* korall_request_body_get(const HTTPRequest* req) {
	return req->body;
}

/*
	Sets the type of response
*/
int korall_response_start_set(HTTPResponse* res, HTTPStatusCode code) {
	const char* reason_phrase = lookup_int_str(code, &http_status_code_lookup_table);
	if (reason_phrase == NULL) {
		logger(LOG_ERR, "failed to set response, invalid code\n");
		return -1;
	}
	String start_line = res->start_line;
	char* str = start_line.chars;

	sprintf(str, "HTTP/1.1 %d %s\r\n", code, reason_phrase);

	// add some headers

	// date
	char date[MAX_DATE_STR_LEN + 1] = { 0 };
	String s = { .chars = date, .size = MAX_DATE_STR_LEN + 1 }; // + 1 needed, \0 included for strftime
	http_get_current_date(&s);
	if (korall_response_header_set(res, "Date", date) == -1) return -1;

	// server
	if (korall_response_header_set(res, "Server", SERVER_SOFTWARE) == -1) return -1;

	return 0;
}

/*
	Sets the value of a header of the response
*/
int korall_response_header_set(HTTPResponse* res, const char* field, const char* value) {
	if (field == NULL || value == NULL) {
		logger(LOG_ERR, "failed to set header, field and value must not be NULL\n");
		return -1;
	}
	
	if (res->header_count >= res->header_capacity) {
		logger(LOG_ERR, "failed to set header, maximum of %zu headers reached\n", res->header_capacity);
		return -1;
	}

	String headers = res->headers;
	char* str = headers.chars;

	size_t field_len = strlen(field);
	size_t value_len = strlen(value);

	if (field_len + value_len + 4 > res->header_size) {
		logger(LOG_ERR, "failed to set header, must be under %zu characters total\n", res->header_size);
		return -1;
	}

	HTTPResponseHeaderField res_field = lookup_str_int(field, &http_res_header_field_lookup_table, true);
	if (http_process_response_header_value(res_field, value) != HTTP_SUCCESS) {
		logger(LOG_ERR, "failed to set header, value is not valid for field %s\n", field);
		return -1;
	}

	res->headers.chars += sprintf(str, "%s: %s\r\n", field, value);
	res->header_count++;
	return 0;
}

/*
	Sets the body of the reponse
*/
int korall_response_body_set(HTTPResponse* res, const char* body) {
	if (body == NULL) return -1;

	size_t body_len = strlen(body);
	if (body_len > res->body.size) {
		logger(LOG_ERR, "failed to set body, body too long\n");
		return -1;
	}

	char* str = res->body.chars;

	// content length
	char cl[MAX_HTTP_BODY_DIGIT_LEN + 1] = { 0 };
	sprintf(cl, "%" PRIu64, body_len);
	if (korall_response_header_set(res, "Content-Length", cl) == -1) return -1;
	
	// body
	sprintf(str, "\r\n%s", body);
	return 0;
}
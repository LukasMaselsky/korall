#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#include "arena.h"
#elif defined TESTS

TEST("http_validate_request") {
	
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest *req = http_request_init(&arena);
	int res;
	char* str;

	str = "GET / HTTP/1.1\r\n\r\n";
	res = http_validate_request(str, strlen(str), req);
	ASSERT(res == 0);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
	res = http_validate_request(str, strlen(str), req);
	ASSERT(res == 0);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1\r\nHost: \r\n\r\n";
	res = http_validate_request(str, strlen(str), req);
	ASSERT(res == -1);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1";
	res = http_validate_request(str, strlen(str), req);
	ASSERT(res == -1);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1\r\n";
	res = http_validate_request(str, strlen(str), req);
	ASSERT(res == -1);


	
	
	
	http_request_free(&arena, req);
}

TEST("http_process_header") {

}

TEST("http_process_headers") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "Host: localhost\r\n\r\n";
	res = http_process_headers(&str, req);
	ASSERT(res == 0);

	http_request_clear(&arena, &req);
	str = "Host: localhost\r\n";
	res = http_process_headers(&str, req);
	ASSERT(res == -1);

	http_request_clear(&arena, &req);
	str = "Host:      \r\n\r\n";
	res = http_process_headers(&str, req);
	ASSERT(res == -1);

	http_request_clear(&arena, &req);
	str = "FakeField: localhost\r\n";
	res = http_process_headers(&str, req);
	ASSERT(res == -1);

	http_request_free(&arena, req);
}


TEST("http_process_request_target") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest *req = http_request_init(&arena);
	HTTPMethod method;
	int res;

	const char* str = "* HTTP/1.1";
	req->start_line->method = HTTP_OPTIONS;
	res = http_process_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "*") == 0);

	http_request_clear(&arena, &req);
	str = "/a/b/c HTTP/1.1";
	req->start_line->method = HTTP_GET;
	res = http_process_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/a/b/c") == 0);

	http_request_clear(&arena, &req);
	str = "/ HTTP/1.1\r\n";
	req->start_line->method = HTTP_GET;
	res = http_process_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/") == 0);

	http_request_clear(&arena, &req);
	str = "localhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = http_process_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "localhost:3500") == 0);

	http_request_clear(&arena, &req);
	str = "localhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = http_process_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "localhost:3500") == 0);

	// connect, rt too long
	http_request_clear(&arena, &req);
	str = "localhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = http_process_request_target(&str, req);
	ASSERT(res == -1);

	// todo: absolute paths

	http_request_free(&arena, req);
}

TEST("http_process_request_target_relative") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest *req = http_request_init(&arena);
	
	int res;
	const char* str;

	str = "/a/b/c HTTP/1.1";
	res = http_process_request_target_relative(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/a/b/c") == 0);

	http_request_clear(&arena, &req);
	str = "/ HTTP/1.1";
	res = http_process_request_target_relative(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/") == 0);

	http_request_clear(&arena, &req);
	str = "// HTTP/1.1";
	res = http_process_request_target_relative(&str, req);
	ASSERT(res == -1);

	http_request_clear(&arena, &req);
	str = "/{/a HTTP/1.1";
	res = http_process_request_target_relative(&str, req);
	ASSERT(res == -1);


	http_request_free(&arena, req);

}

TEST("http_process_protocol") {
	const char* str = "HTTP/1.1";
	int res;
	res = http_process_protocol(&str);
	ASSERT(res == -1);

	str = " HTTP/1.1";
	res = http_process_protocol(&str);
	ASSERT(res == -1);

	str = "HTTP/1.";
	res = http_process_protocol(&str);
	ASSERT(res == -1);

	str = "HTTP/1.1\r\n";
	res = http_process_protocol(&str);
	ASSERT(res == 0);

	str = "HTTP/1.11\r\n";
	res = http_process_protocol(&str);
	ASSERT(res == -1);

	str = "";
	res = http_process_protocol(&str);
	ASSERT(res == -1);

	str = " ";
	res = http_process_protocol(&str);
	ASSERT(res == -1);

}

TEST("http_process_method") {
	const int table_len = HTTP_METHOD_LOOKUP_TABLE_COUNT;

	const char* str = "POST /users HTTP/1.1";
	HTTPMethod res;
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_POST);
	str = "CONNECT /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_CONNECT);
	str = "DELETE /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_DELETE);
	str = "GET /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_GET);
	str = "PUT /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_PUT);
	str = "TRACE /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_TRACE);
	str = "PATCH /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_PATCH);
	str = "OPTIONS /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_OPTIONS);
	str = "HEAD /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_HEAD);

	str = "GE /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_BADMETHOD);

	str = "get /users HTTP/1.1";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_BADMETHOD);

	str = "GET";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_BADMETHOD);
	
	str = "GET ";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_GET);

	str = "";
	res = http_process_method(&str, http_method_lookup_table, table_len);
	ASSERT(res == HTTP_BADMETHOD);
}

TEST("http_get_current_date") {
	char buf[MAX_DATE_STR_LEN + 1];
	http_get_current_date(buf, MAX_DATE_STR_LEN + 1);
	// todo
}

TEST("http_response_to_str") {
	Arena arena = arena_init(HTTP_RES_SIZE);

	HTTPResponse* res = http_response_construct(&arena, HTTP_SC_200, "MyServer", HTTP_MT_TXT_PLAIN, "Hello World!");
	char *r = http_response_to_str(res);
	ASSERT(starts_with("HTTP/1.1 200 OK", r));
	ASSERT(r != NULL);
	free(r);


}


#endif
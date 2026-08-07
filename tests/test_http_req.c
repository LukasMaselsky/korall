#if defined HEADERS
#include "http/http_internal.h"
#include "lookup/lookup_tables.h"
#include "arena/arena.h"
#elif defined TESTS

TEST("http_request_parse")
{

	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);
	HTTPError res;
	char *str;

	str = "GET / HTTP/1.1\r\n\r\n";
	res = http_request_parse(str, req);
	ASSERT(res == HTTP_SUCCESS);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
	res = http_request_parse(str, req);
	ASSERT(res == HTTP_SUCCESS);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1\r\nHost: \r\n\r\n";
	res = http_request_parse(str, req);
	ASSERT(res == HTTP_BAD_DOMAIN_PORT);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1";
	res = http_request_parse(str, req);
	ASSERT(res == HTTP_BAD_PROT);

	http_request_clear(&arena, &req);
	str = "GET / HTTP/1.1\r\n";
	res = http_request_parse(str, req);
	ASSERT(res == HTTP_BAD_HEADER);

	arena_free(&arena);
}

TEST("http_process_header")
{
}

TEST("http_process_headers")
{
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);
	int res;
	const char *str;

	str = "Host: localhost\r\n\r\n";
	res = http_request_process_headers(&str, req);
	ASSERT(res == HTTP_SUCCESS);

	http_request_clear(&arena, &req);
	str = "Host: localhost\r\n";
	res = http_request_process_headers(&str, req);
	ASSERT(res == HTTP_BAD_HEADER);

	http_request_clear(&arena, &req);
	str = "Host:      \r\n\r\n";
	res = http_request_process_headers(&str, req);
	ASSERT(res == HTTP_BAD_DOMAIN_PORT);

	http_request_clear(&arena, &req);
	str = "FakeField: localhost\r\n";
	res = http_request_process_headers(&str, req);
	ASSERT(res == HTTP_BAD_HEADER);

	arena_free(&arena);
}

TEST("http_request_process_target")
{
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);
	HTTPMethod method;
	int res;

	const char *str = "* HTTP/1.1";
	req->start_line->method = HTTP_OPTIONS;
	res = http_request_process_target(&str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "*") == 0);

	http_request_clear(&arena, &req);
	str = "/a/b/c HTTP/1.1";
	req->start_line->method = HTTP_GET;
	res = http_request_process_target(&str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/a/b/c") == 0);

	http_request_clear(&arena, &req);
	str = "/ HTTP/1.1\r\n";
	req->start_line->method = HTTP_GET;
	res = http_request_process_target(&str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/") == 0);

	http_request_clear(&arena, &req);
	str = "localhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = http_request_process_target(&str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "localhost:3500") == 0);

	http_request_clear(&arena, &req);
	str = "localhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = http_request_process_target(&str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "localhost:3500") == 0);

	// connect, rt too long
	http_request_clear(&arena, &req);
	str = "localhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = http_request_process_target(&str, req);
	ASSERT(res == HTTP_REQUEST_TARGET_TOO_BIG);

	// todo: absolute paths

	arena_free(&arena);
}

TEST("http_request_process_target_relative")
{
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);

	int res;
	const char *str;

	str = "/a/b/c";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->start_line->request_target, "/a/b/c") == 0);

	http_request_clear(&arena, &req);
	str = "/";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->start_line->request_target, "/") == 0);

	http_request_clear(&arena, &req);
	str = "//";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "//a/a";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field=val";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->start_line->request_target, "/a?field=val") == 0);

	http_request_clear(&arena, &req);
	str = "/a?field=val&";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field=val&";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field==val";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field=val&&field=val";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field=v%AA";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_SUCCESS);

	http_request_clear(&arena, &req);
	str = "/a?field=v%A";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "/a?field=v,b";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_SUCCESS);

	http_request_clear(&arena, &req);
	str = "/a?field=v,";
	res = http_request_process_target_relative(str, req);
	ASSERT(res == HTTP_SUCCESS);

	// todo: more tests

	arena_free(&arena);
}

TEST("http_request_process_target_absolute")
{
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);

	int res;
	const char *str;

	str = "http://www.example.re/page";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->start_line->request_target, "http://www.example.re/page") == 0);

	http_request_clear(&arena, &req);
	str = "http://www.example.re";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->start_line->request_target, "http://www.example.re") == 0);

	http_request_clear(&arena, &req);
	str = "http://www.example.re";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->start_line->request_target, "http://www.example.re") == 0);

	http_request_clear(&arena, &req);
	str = "http://www.example..re";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "http://example.re./a";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "http://example.re-/a";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "http://example.-re/a";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	http_request_clear(&arena, &req);
	str = "http://www.-example.re/a";
	res = http_request_process_target_absolute(str, req);
	ASSERT(res == HTTP_BAD_REQUEST_TARGET);

	arena_free(&arena);
}

TEST("http_process_protocol")
{
	const char *str = "HTTP/1.1";
	int res;
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_BAD_PROT);

	str = " HTTP/1.1";
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_BAD_PROT);

	str = "HTTP/1.";
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_BAD_PROT);

	str = "HTTP/1.1\r\n";
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_SUCCESS);

	str = "HTTP/1.11\r\n";
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_BAD_PROT);

	str = "";
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_BAD_PROT);

	str = " ";
	res = http_request_process_protocol(&str);
	ASSERT(res == HTTP_BAD_PROT);
}

TEST("http_process_method")
{
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);

	const char *str = "POST /users HTTP/1.1";
	HTTPError res;

	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_POST);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "CONNECT /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_CONNECT);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "DELETE /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_DELETE);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "GET /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_GET);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "PUT /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_PUT);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "TRACE /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_TRACE);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "PATCH /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_PATCH);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "OPTIONS /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_OPTIONS);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "HEAD /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_HEAD);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "GE /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(res == HTTP_BAD_METHOD);
	http_request_clear(&arena, &req);

	str = "get /users HTTP/1.1";
	res = http_request_process_method(&str, req);
	ASSERT(res == HTTP_BAD_METHOD);
	http_request_clear(&arena, &req);

	str = "GET";
	res = http_request_process_method(&str, req);
	ASSERT(res == HTTP_BAD_METHOD);
	http_request_clear(&arena, &req);

	str = "GET ";
	res = http_request_process_method(&str, req);
	ASSERT(req->start_line->method == HTTP_GET);
	ASSERT(res == HTTP_SUCCESS);
	http_request_clear(&arena, &req);

	str = "";
	res = http_request_process_method(&str, req);
	ASSERT(res == HTTP_BAD_METHOD);
	http_request_clear(&arena, &req);
}

TEST("http_get_current_date")
{
	const char buf[MAX_DATE_STR_LEN + 1];
	String str = {.chars = buf, .size = MAX_DATE_STR_LEN + 1};
	http_get_current_date(&str);
	// todo
}

TEST("http_verify_origin")
{

	// todo
}

TEST("http_allowed_methods")
{
	int data[100] = {0};
	Array arr = array_create_stack(data, sizeof(int), 0, 30);
	int res;

	res = http_allowed_methods(NULL, NULL, 0);
	ASSERT(res == -1);
	res = http_allowed_methods(&arr, NULL, 0);
	ASSERT(res == -1);

	HTTPMethod m = HTTP_GET;
	array_push(&arr, &m);
	m = HTTP_POST;
	array_push(&arr, &m);

	res = http_allowed_methods(&arr, NULL, 0);
	ASSERT(res == -1);
	char temp[1000] = {0};
	res = http_allowed_methods(&arr, temp, 999);
	ASSERT(res == 0);
	ASSERT(strcmp(temp, "GET,POST") == 0);
}

TEST("http_allowed_headers")
{
	int data[1000] = {0};
	Array arr = array_create_stack(data, sizeof(char *), 0, 30);
	int res;

	res = http_allowed_headers(NULL, NULL, 0);
	ASSERT(res == -1);
	res = http_allowed_headers(&arr, NULL, 0);
	ASSERT(res == -1);

	char *m = "x";
	array_push(&arr, &m);
	m = "y";
	array_push(&arr, &m);

	res = http_allowed_headers(&arr, NULL, 0);
	ASSERT(res == -1);
	char temp[1000] = {0};
	res = http_allowed_headers(&arr, temp, 999);
	ASSERT(res == 0);
	ASSERT(strcmp(temp, "x,y") == 0);
}

#endif
#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#include "http_headers.h"
#include "array.h"
#elif defined TESTS

TEST("http_process_host") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "localhost:3500";
	res = http_process_host(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->headers->host->domain, "localhost") == 0);
	ASSERT(strcmp(req->headers->host->port, "3500") == 0);

	
	// port too small
	http_request_clear(&arena, &req);
	str = "localhost:88";
	res = http_process_host(str, req);
	ASSERT(res == HTTP_BAD_PORT);
}

TEST("http_process_accept") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "application/json";
	res = http_process_accept(str, req);
	ASSERT(res == HTTP_SUCCESS);
	
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 0))->field == HTTP_MT_APP_JSON);
	http_request_clear(&arena, &req);

	str = "app/json";
	res = http_process_accept(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT);
	http_request_clear(&arena, &req);

	str = "text/html, text/plain;q=0.9, text/*;q=0.8, */*;q=0.7";
	res = http_process_accept(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 0))->field == HTTP_MT_TXT_HTML);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 1))->field == HTTP_MT_TXT_PLAIN);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 2))->field == HTTP_MT_TXT);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 3))->field == HTTP_MT_ANY);
	http_request_clear(&arena, &req);

	str = "text/html,text/plain;q=0.9,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 0))->field == HTTP_MT_TXT_HTML);
	http_request_clear(&arena, &req);

	str = "text/html,text/plain;q=0.,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT);
	http_request_clear(&arena, &req);

	str = "text/html,text/plain;q=0,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT);
	http_request_clear(&arena, &req);

	
}

TEST("http_process_accept_encoding") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "gzip";
	res = http_process_accept_encoding(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept_encoding, 0))->field == HTTP_ENC_GZIP);
	http_request_clear(&arena, &req);

	str = "gzi";
	res = http_process_accept_encoding(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT_ENC);
	http_request_clear(&arena, &req);

	str = "gzip, deflate;q=0.9";
	res = http_process_accept_encoding(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept_encoding, 0))->field == HTTP_ENC_GZIP);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept_encoding, 1))->field == HTTP_ENC_DEFLATE);
	http_request_clear(&arena, &req);

}

TEST("http_process_content_length") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "10000";
	res = http_process_content_length(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->content_length == 10000);
	http_request_clear(&arena, &req);

	str = "10000a";
	res = http_process_content_length(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	http_request_clear(&arena, &req);

	str = "";
	res = http_process_content_length(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	http_request_clear(&arena, &req);

	str = " ";
	res = http_process_content_length(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	http_request_clear(&arena, &req);

}

TEST("http_process_content_type") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "text/html";
	res = http_process_content_type(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->content_type->media_type == HTTP_MT_TXT_HTML);
	http_request_clear(&arena, &req);

	str = "text/html; charset=UTF-8";
	res = http_process_content_type(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->content_type->media_type == HTTP_MT_TXT_HTML);
	ASSERT(req->headers->content_type->charset == HTTP_CHS_UTF_8);
	http_request_clear(&arena, &req);

	str = "text/html; charsets=UTF-8";
	res = http_process_content_type(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	http_request_clear(&arena, &req);

	str = "";
	res = http_process_content_type(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	http_request_clear(&arena, &req);

	str = " ";
	res = http_process_content_type(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	http_request_clear(&arena, &req);

}

TEST("http_process_access_control_request_method") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "GET";
	res = http_process_access_control_request_method(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->access_control_request_method == HTTP_GET);
	http_request_clear(&arena, &req);

	str = "OPTIONS";
	res = http_process_access_control_request_method(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->access_control_request_method == HTTP_OPTIONS);
	http_request_clear(&arena, &req);

	str = "POST";
	res = http_process_access_control_request_method(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->access_control_request_method == HTTP_POST);
	http_request_clear(&arena, &req);

}

TEST("http_process_access_control_request_headers") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	Array* arr = req->headers->access_control_request_headers;

	str = "content-type, a-im";
	res = http_process_access_control_request_headers(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 2);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 0)) == HTTP_RQH_CONTENT_TYPE);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 1)) == HTTP_RQH_A_IM);
	http_request_clear(&arena, &req);

	str = "content-type,a-im";
	res = http_process_access_control_request_headers(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 2);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 0)) == HTTP_RQH_CONTENT_TYPE);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 1)) == HTTP_RQH_A_IM);
	http_request_clear(&arena, &req);

	str = "content-type";
	res = http_process_access_control_request_headers(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 1);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 0)) == HTTP_RQH_CONTENT_TYPE);
	http_request_clear(&arena, &req);

	str = "content-typ";
	res = http_process_access_control_request_headers(str, req);
	ASSERT(res == HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS);
	ASSERT(arr->size == 0);
	http_request_clear(&arena, &req);

}

TEST("http_process_connection") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "keep-alive";
	res = http_process_connection(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->connection == HTTP_CON_KEEP_ALIVE);
	http_request_clear(&arena, &req);

	str = "close";
	res = http_process_connection(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->connection == HTTP_CON_CLOSE);
	http_request_clear(&arena, &req);

	str = "clos";
	res = http_process_connection(str, req);
	ASSERT(res == HTTP_BAD_CONNECTION);
	http_request_clear(&arena, &req);

}

#endif
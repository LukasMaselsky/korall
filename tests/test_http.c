#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#elif defined TESTS

TEST("validate_http_request") {
	char req_t[MAX_HTTP_URL_LEN];
	HTTPRequest req;
	req.request_target = req_t;
	int res;
	const char* str;

	str = "GET / HTTP/1.1\n";
	res = validate_http_request(str, strlen(str), &req);
	ASSERT(res == 0);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	str = "GET / HTTP/1.1";
	res = validate_http_request(str, strlen(str), &req);
	ASSERT(res == -1);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	

}

TEST("process_http_request_target") {

	char req_t[MAX_HTTP_URL_LEN];
	HTTPRequest req;
	req.request_target = req_t;

	HTTPMethod method;
	int res;

	const char* str = "* HTTP/1.1";
	method = HTTP_OPTIONS;
	res = process_http_request_target(&str, method, &req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req.request_target, "*") == 0);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	str = "/a/b/c HTTP/1.1";
	method = HTTP_GET;
	res = process_http_request_target(&str, method, &req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req.request_target, "/a/b/c") == 0);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	str = "/ HTTP/1.1\n";
	method = HTTP_GET;
	res = process_http_request_target(&str, method, &req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1\n") == 0);
	ASSERT(strcmp(req.request_target, "/") == 0);

	// todo: absolute paths
}

TEST("process_http_request_target_relative") {
	char req_t[MAX_HTTP_URL_LEN];
	HTTPRequest req;
	req.request_target = req_t;

	int res;
	const char* str = "/a/b/c HTTP/1.1";
	res = process_http_request_target_relative(&str, &req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req.request_target, "/a/b/c") == 0);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	str = "/ HTTP/1.1";
	res = process_http_request_target_relative(&str, &req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req.request_target, "/") == 0);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	str = "// HTTP/1.1";
	res = process_http_request_target_relative(&str, &req);
	ASSERT(res == -1);

	memset(req.request_target, 0, MAX_HTTP_URL_LEN);
	str = "/{/a HTTP/1.1";
	res = process_http_request_target_relative(&str, &req);
	ASSERT(res == -1);


}

TEST("process_http_protocol") {
	const char* str = "HTTP/1.1";
	int res;
	res = process_http_protocol(&str);
	ASSERT(res == 0);

	str = " HTTP/1.1";
	res = process_http_protocol(&str);
	ASSERT(res == -1);

	str = "HTTP/1.";
	res = process_http_protocol(&str);
	ASSERT(res == -1);

	str = "HTTP/1.1 ";
	res = process_http_protocol(&str);
	ASSERT(res == 0);

	str = "HTTP/1.1\n";
	res = process_http_protocol(&str);
	ASSERT(res == 0);

	str = "";
	res = process_http_protocol(&str);
	ASSERT(res == -1);

	str = " ";
	res = process_http_protocol(&str);
	ASSERT(res == -1);

}

TEST("process_http_method") {
	const int table_len = HTTP_METHOD_LOOKUP_TABLE_COUNT;
	const LookupEntry table[HTTP_METHOD_LOOKUP_TABLE_COUNT] = {
		{ "CONNECT", HTTP_CONNECT },
		{ "DELETE", HTTP_DELETE },
		{ "GET", HTTP_GET },
		{ "HEAD", HTTP_HEAD },
		{ "OPTIONS", HTTP_OPTIONS },
		{ "PATCH", HTTP_PATCH },
		{ "POST", HTTP_POST },
		{ "PUT", HTTP_PUT },
		{ "TRACE" , HTTP_TRACE }
	};
	const char* str = "POST /users HTTP/1.1";
	HTTPMethod res;
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_POST);
	str = "CONNECT /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_CONNECT);
	str = "DELETE /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_DELETE);
	str = "GET /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_GET);
	str = "PUT /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_PUT);
	str = "TRACE /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_TRACE);
	str = "PATCH /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_PATCH);
	str = "OPTIONS /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_OPTIONS);
	str = "HEAD /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_HEAD);

	str = "GE /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_BADMETHOD);

	str = "get /users HTTP/1.1";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_BADMETHOD);

	str = "GET";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_GET);

	str = "";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_BADMETHOD);
}


#endif
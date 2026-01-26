#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#elif defined TESTS

TEST("validate_http_request") {
	
	HTTPRequest *req = http_request_init();
	int res;
	char* str;

	str = "GET / HTTP/1.1\r\n\r\n";
	res = validate_http_request(str, strlen(str), req);
	ASSERT(res == 0);

	http_request_clear(&req);
	str = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
	res = validate_http_request(str, strlen(str), req);
	ASSERT(res == 0);

	http_request_clear(&req);
	str = "GET / HTTP/1.1\r\nHost: \r\n\r\n";
	res = validate_http_request(str, strlen(str), req);
	ASSERT(res == -1);

	http_request_clear(&req);
	str = "GET / HTTP/1.1";
	res = validate_http_request(str, strlen(str), req);
	ASSERT(res == -1);

	http_request_clear(&req);
	str = "GET / HTTP/1.1\r\n";
	res = validate_http_request(str, strlen(str), req);
	ASSERT(res == -1);


	
	
	
	http_request_free(req);
}

TEST("process_http_header") {

}

TEST("process_http_headers") {
	HTTPRequest* req = http_request_init();
	int res;
	const char* str;

	str = "Host: localhost\r\n\r\n";
	res = process_http_headers(&str, req);
	ASSERT(res == 0);

	http_request_clear(&req);
	str = "Host: localhost\r\n";
	res = process_http_headers(&str, req);
	ASSERT(res == -1);

	http_request_clear(&req);
	str = "Host:      \r\n\r\n";
	res = process_http_headers(&str, req);
	ASSERT(res == -1);

	http_request_clear(&req);
	str = "FakeField: localhost\r\n";
	res = process_http_headers(&str, req);
	ASSERT(res == -1);

	http_request_free(req);
}


TEST("process_http_request_target") {

	HTTPRequest *req = http_request_init();
	HTTPMethod method;
	int res;

	const char* str = "* HTTP/1.1";
	req->start_line->method = HTTP_OPTIONS;
	res = process_http_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "*") == 0);

	http_request_clear(&req);
	str = "/a/b/c HTTP/1.1";
	req->start_line->method = HTTP_GET;
	res = process_http_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/a/b/c") == 0);

	http_request_clear(&req);
	str = "/ HTTP/1.1\r\n";
	req->start_line->method = HTTP_GET;
	res = process_http_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/") == 0);

	http_request_clear(&req);
	str = "localhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = process_http_request_target(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1\r\n") == 0);
	ASSERT(strcmp(req->start_line->request_target, "localhost:3500") == 0);

	// connect, rt too long
	http_request_clear(&req);
	str = "localhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhostlocalhost:3500 HTTP/1.1\r\n";
	req->start_line->method = HTTP_CONNECT;
	res = process_http_request_target(&str, req);
	ASSERT(res == -1);

	// todo: absolute paths

	http_request_free(req);
}

TEST("process_http_request_target_relative") {
	HTTPRequest *req = http_request_init();
	
	int res;
	const char* str;

	str = "/a/b/c HTTP/1.1";
	res = process_http_request_target_relative(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/a/b/c") == 0);

	http_request_clear(&req);
	str = "/ HTTP/1.1";
	res = process_http_request_target_relative(&str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(str, " HTTP/1.1") == 0);
	ASSERT(strcmp(req->start_line->request_target, "/") == 0);

	http_request_clear(&req);
	str = "// HTTP/1.1";
	res = process_http_request_target_relative(&str, req);
	ASSERT(res == -1);

	http_request_clear(&req);
	str = "/{/a HTTP/1.1";
	res = process_http_request_target_relative(&str, req);
	ASSERT(res == -1);


	http_request_free(req);

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

	str = "HTTP/1.1\r\n";
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
	const LookupEntryStrInt table[HTTP_METHOD_LOOKUP_TABLE_COUNT] = {
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

TEST("http_get_current_date") {
	char buf[MAX_DATE_STR_LEN + 1];
	http_get_current_date(buf, MAX_DATE_STR_LEN + 1);
	// todo
}

TEST("http_response_to_str") {
	HTTPResponse* res = http_response_construct(HTTP_SC_200, "MyServer", MT_TXT_PLAIN, "Hello World!");
	char *r = http_response_to_str(res);
	ASSERT(r != NULL);
	free(r);


}


#endif
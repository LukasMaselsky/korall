#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#elif defined TESTS

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
	ASSERT(res == -1);

	str = "HTTP/1.1\n";
	res = process_http_protocol(&str);
	ASSERT(res == -1);

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
	bool res;
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
	ASSERT(res == HTTP_BADMETHOD);

	str = "";
	res = process_http_method(&str, &table, table_len);
	ASSERT(res == HTTP_BADMETHOD);
}


#endif
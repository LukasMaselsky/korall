#if defined HEADERS
#include "http/http.h"
#include "lookup/lookup_tables.h"
#include "arena/arena.h"
#elif defined TESTS

TEST("korall_request_header_get")
{
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest *req = http_request_init(&arena);

	char value[1000] = {0};
	char *str;
	int res;

	str = "Host: localhost:3500\r\nUser-Agent: curl / 8.17.0\r\nAccept: */*\r\nOrigin: http://localhost:3000\r\nAccess-Control-Request-Method: POST";
	req->headers = str;

	res = korall_request_header_get(req, "Accept", value, 999);
	ASSERT(res == 0);
	ASSERT(strcmp(value, "*/*") == 0);

	memset(value, 0, 1000);

	res = korall_request_header_get(req, "Host", value, 999);
	ASSERT(res == 0);
	ASSERT(strcmp(value, "localhost:3500") == 0);

	memset(value, 0, 1000);

	res = korall_request_header_get(req, "Access-Control-Request-Method", value, 999);
	ASSERT(res == 0);
	ASSERT(strcmp(value, "POST") == 0);

	memset(value, 0, 1000);

	res = korall_request_header_get(req, "Access-Control-Request-Method", NULL, 0);
	ASSERT(res == 0);
	ASSERT(strcmp(value, "") == 0);

	arena_free(&arena);
}

// todo

#endif
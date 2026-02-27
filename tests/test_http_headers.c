#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#elif defined TESTS

TEST("http_process_host") {
	HTTPRequest* req = http_request_init();
	int res;
	const char* str;

	str = "localhost:3500";
	res = http_process_host(str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(req->headers->host->domain, "localhost") == 0);
	ASSERT(strcmp(req->headers->host->port, "3500") == 0);

	
	// port too small
	http_request_clear(&req);
	str = "localhost:88";
	res = http_process_host(str, req);
	ASSERT(res == -1);
}

TEST("http_process_accept") {
	HTTPRequest* req = http_request_init();
	int res;
	const char* str;

	str = "application/json";
	res = http_process_accept(str, req);
	ASSERT(res == 0);
	ASSERT(req->headers->accept == HTTP_MT_APP_JSON);
	http_request_clear(&req);

	str = "app/json";
	res = http_process_accept(str, req);
	ASSERT(res == -1);
	http_request_clear(&req);


	
}

#endif
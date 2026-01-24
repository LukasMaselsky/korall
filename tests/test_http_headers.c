#if defined HEADERS
#include "http_.h"
#include "lookup_tables.h"
#elif defined TESTS

TEST("process_http_host") {
	HTTPRequest* req = http_request_st_init();
	int res;
	const char* str;

	str = "localhost:3500";
	res = process_http_host(str, req);
	ASSERT(res == 0);
	ASSERT(strcmp(req->headers->host->domain, "localhost") == 0);
	ASSERT(strcmp(req->headers->host->port, "3500") == 0);

	
	// port too small
	http_request_st_clear(&req);
	str = "localhost:88";
	res = process_http_host(str, req);
	ASSERT(res == -1);
}

#endif
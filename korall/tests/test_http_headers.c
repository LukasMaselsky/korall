#if defined HEADERS
#include "http_internal.h"
#include "lookup_tables.h"
#include "http_headers.h"
#include "array.h"
#elif defined TESTS

TEST("http_process_host") {
	Arena arena = arena_init(HTTP_REQ_ARENA_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "localhost:3500";
	res = http_process_host(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->host->domain, "localhost") == 0);
	ASSERT(strcmp(req->host->port, "3500") == 0);

	
	// port too small
	http_request_clear(&arena, &req);
	str = "localhost:88";
	res = http_process_host(str, req);
	ASSERT(res == HTTP_BAD_PORT);
}

TEST("http_process_accept") {
	
	const char* str;
	int res;

	str = "application/json";
	res = http_process_accept(str);
	ASSERT(res == HTTP_SUCCESS);

	str = "app/json";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);

	str = "text/html, text/plain;q=0.9, text/*;q=0.8, */*;q=0.7";
	res = http_process_accept(str);
	ASSERT(res == HTTP_SUCCESS);

	str = "text/plain;q=1";
	res = http_process_accept(str);
	ASSERT(res == HTTP_SUCCESS);

	str = "text/html,text/plain;q=0.9,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept(str);
	ASSERT(res == HTTP_SUCCESS);

	str = "text/html,text/plain;q=0.,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);
	

	str = "text/html,text/plain;q=0,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);
	

	str = "text/html,text/plain;q=0,text/*;q=0.8,*/*;q=0.";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);
	

	str = "text/html,text/plain;q=0,text/*;q=0.8,*/*;q=0";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);
	

	str = "text/plain;q=1.";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);
	

	str = "text/plain;q=1,";
	res = http_process_accept(str);
	ASSERT(res == HTTP_BAD_ACCEPT);
	
	
}

TEST("http_process_accept_encoding") {
	const char* str;
	int res;

	str = "gzip";
	res = http_process_accept_encoding(str);
	ASSERT(res == HTTP_SUCCESS);

	str = "gzi";
	res = http_process_accept_encoding(str);
	ASSERT(res == HTTP_BAD_ACCEPT_ENC);

	str = "gzip, deflate;q=0.9";
	res = http_process_accept_encoding(str);
	ASSERT(res == HTTP_SUCCESS);

}

TEST("http_process_content_length") {
	
	const char* str;
	int res;

	str = "10000";
	res = http_process_content_length(str);
	ASSERT(res == HTTP_SUCCESS);

	str = "10000a";
	res = http_process_content_length(str);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);

	str = "";
	res = http_process_content_length(str);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	

	str = " ";
	res = http_process_content_length(str);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	

}

TEST("http_process_content_type") {
	
	const char* str;
	int res;

	str = "text/html";
	res = http_process_content_type(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "text/html; charset=UTF-8";
	res = http_process_content_type(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "text/html; charsets=UTF-8";
	res = http_process_content_type(str);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	

	str = "";
	res = http_process_content_type(str);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	

	str = " ";
	res = http_process_content_type(str);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	

}

TEST("http_process_access_control_request_method") {
	
	const char* str;
	int res;

	str = "GET";
	res = http_process_access_control_request_method(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "OPTIONS";
	res = http_process_access_control_request_method(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "POST";
	res = http_process_access_control_request_method(str);
	ASSERT(res == HTTP_SUCCESS);
	

}

TEST("http_process_access_control_request_headers") {
	
	const char* str;
	int res;

	str = "content-type, a-im";
	res = http_process_access_control_request_headers(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "content-type,a-im";
	res = http_process_access_control_request_headers(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "content-type";
	res = http_process_access_control_request_headers(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "content-typ";
	res = http_process_access_control_request_headers(str);
	ASSERT(res == HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS);
	

}

TEST("http_process_connection") {
	
	const char* str;
	int res;

	str = "keep-alive";
	res = http_process_connection(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "close";
	res = http_process_connection(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "clos";
	res = http_process_connection(str);
	ASSERT(res == HTTP_BAD_CONNECTION);
	

}

TEST("http_process_cache_control_req") {

	const char* str;
	int res;

	str = "no-transform";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "min-fresh=3600";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "min-fresh=3600, no-transform";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "no-transform, min-fresh=3600";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "no-transform, min-fresh=3600a";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_BAD_CACHE_CONTROL);
	

	str = "no-transform=3600, min-fresh=3600";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_BAD_CACHE_CONTROL);
	

	str = "";
	res = http_process_cache_control_req(str);
	ASSERT(res == HTTP_BAD_CACHE_CONTROL);
	


}

TEST("http_process_user_agent") {

	const char* str;
	int res;

	str = "Mozilla/5.0 (X11; Linux x86_64; rv:12.0) Gecko/20100101 Firefox/12.0";
	res = http_process_user_agent(str);
	ASSERT(res == HTTP_SUCCESS);
	

}

TEST("http_process_date") {

	const char* str;
	int res;

	str = "Tue, 29 Oct 2024 16:56:32 GMT";
	res = http_process_date(str);
	ASSERT(res == HTTP_SUCCESS);

	

	str = "Thu, 05 Mar 2026 16:40:31 GMT";
	res = http_process_date(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "Thu,";
	res = http_process_date(str);
	ASSERT(res == HTTP_BAD_DATE);
	
}

TEST("http_process_expect") {
	
	const char* str;
	int res;

	str = "100-continue";
	res = http_process_expect(str);
	ASSERT(res == HTTP_SUCCESS);
	

	str = "100";
	res = http_process_expect(str);
	ASSERT(res == HTTP_BAD_EXPECT);
	

}

#endif
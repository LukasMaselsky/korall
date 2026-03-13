#if defined HEADERS
#include "http_internal.h"
#include "lookup_tables.h"
#include "http_headers.h"
#include "array.h"
#elif defined TESTS

TEST("http_process_host_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "localhost:3500";
	res = http_process_host_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(req->headers->host->domain, "localhost") == 0);
	ASSERT(strcmp(req->headers->host->port, "3500") == 0);

	
	// port too small
	http_request_clear(&arena, &req);
	str = "localhost:88";
	res = http_process_host_req(str, req);
	ASSERT(res == HTTP_BAD_PORT);
}

TEST("http_process_accept_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "application/json";
	res = http_process_accept_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 0))->field == HTTP_MT_APP_JSON);
	http_request_clear(&arena, &req);

	str = "app/json";
	res = http_process_accept_req(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT);
	http_request_clear(&arena, &req);

	str = "text/html, text/plain;q=0.9, text/*;q=0.8, */*;q=0.7";
	res = http_process_accept_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 0))->field == HTTP_MT_TXT_HTML);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 1))->field == HTTP_MT_TXT_PLAIN);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 2))->field == HTTP_MT_TXT);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 3))->field == HTTP_MT_ANY);
	http_request_clear(&arena, &req);

	str = "text/html,text/plain;q=0.9,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept, 0))->field == HTTP_MT_TXT_HTML);
	http_request_clear(&arena, &req);

	str = "text/html,text/plain;q=0.,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept_req(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT);
	http_request_clear(&arena, &req);

	str = "text/html,text/plain;q=0,text/*;q=0.8,*/*;q=0.7";
	res = http_process_accept_req(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT);
	http_request_clear(&arena, &req);

	
}

TEST("http_process_accept_encoding_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "gzip";
	res = http_process_accept_encoding_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept_encoding, 0))->field == HTTP_ENC_GZIP);
	http_request_clear(&arena, &req);

	str = "gzi";
	res = http_process_accept_encoding_req(str, req);
	ASSERT(res == HTTP_BAD_ACCEPT_ENC);
	http_request_clear(&arena, &req);

	str = "gzip, deflate;q=0.9";
	res = http_process_accept_encoding_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept_encoding, 0))->field == HTTP_ENC_GZIP);
	ASSERT(((HTTPWeightedField*)array_get(req->headers->accept_encoding, 1))->field == HTTP_ENC_DEFLATE);
	http_request_clear(&arena, &req);

}

TEST("http_process_content_length_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "10000";
	res = http_process_content_length_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->content_length == 10000);
	http_request_clear(&arena, &req);

	str = "10000a";
	res = http_process_content_length_req(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	http_request_clear(&arena, &req);

	str = "";
	res = http_process_content_length_req(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	http_request_clear(&arena, &req);

	str = " ";
	res = http_process_content_length_req(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_LENGTH);
	http_request_clear(&arena, &req);

}

TEST("http_process_content_type_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "text/html";
	res = http_process_content_type_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->content_type->media_type == HTTP_MT_TXT_HTML);
	http_request_clear(&arena, &req);

	str = "text/html; charset=UTF-8";
	res = http_process_content_type_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->content_type->media_type == HTTP_MT_TXT_HTML);
	ASSERT(req->headers->content_type->charset == HTTP_CHS_UTF_8);
	http_request_clear(&arena, &req);

	str = "text/html; charsets=UTF-8";
	res = http_process_content_type_req(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	http_request_clear(&arena, &req);

	str = "";
	res = http_process_content_type_req(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	http_request_clear(&arena, &req);

	str = " ";
	res = http_process_content_type_req(str, req);
	ASSERT(res == HTTP_BAD_CONTENT_TYPE);
	http_request_clear(&arena, &req);

}

TEST("http_process_access_control_request_method_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "GET";
	res = http_process_access_control_request_method_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->access_control_request_method == HTTP_GET);
	http_request_clear(&arena, &req);

	str = "OPTIONS";
	res = http_process_access_control_request_method_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->access_control_request_method == HTTP_OPTIONS);
	http_request_clear(&arena, &req);

	str = "POST";
	res = http_process_access_control_request_method_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->access_control_request_method == HTTP_POST);
	http_request_clear(&arena, &req);

}

TEST("http_process_access_control_request_headers_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	Array* arr = req->headers->access_control_request_headers;

	str = "content-type, a-im";
	res = http_process_access_control_request_headers_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 2);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 0)) == HTTP_RQH_CONTENT_TYPE);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 1)) == HTTP_RQH_A_IM);
	http_request_clear(&arena, &req);

	str = "content-type,a-im";
	res = http_process_access_control_request_headers_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 2);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 0)) == HTTP_RQH_CONTENT_TYPE);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 1)) == HTTP_RQH_A_IM);
	http_request_clear(&arena, &req);

	str = "content-type";
	res = http_process_access_control_request_headers_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 1);
	ASSERT(*((HTTPRequestHeaderField*)array_get(arr, 0)) == HTTP_RQH_CONTENT_TYPE);
	http_request_clear(&arena, &req);

	str = "content-typ";
	res = http_process_access_control_request_headers_req(str, req);
	ASSERT(res == HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS);
	ASSERT(arr->size == 0);
	http_request_clear(&arena, &req);

}

TEST("http_process_connection_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "keep-alive";
	res = http_process_connection_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->connection == HTTP_CON_KEEP_ALIVE);
	http_request_clear(&arena, &req);

	str = "close";
	res = http_process_connection_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->connection == HTTP_CON_CLOSE);
	http_request_clear(&arena, &req);

	str = "clos";
	res = http_process_connection_req(str, req);
	ASSERT(res == HTTP_BAD_CONNECTION);
	http_request_clear(&arena, &req);

}

TEST("http_process_cache_control_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;
	HTTPRequestCacheControlPair* pair;

	Array* arr = req->headers->cache_control;

	str = "no-transform";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 1);
	pair = (HTTPRequestCacheControlPair*)array_get(arr, 0);
	ASSERT(pair->name == HTTP_REQ_CC_NO_TRANSFORM);
	ASSERT(pair->seconds == -1);
	http_request_clear(&arena, &req);

	str = "min-fresh=3600";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 1);
	pair = (HTTPRequestCacheControlPair*)array_get(arr, 0);
	ASSERT(pair->name == HTTP_REQ_CC_MIN_FRESH);
	ASSERT(pair->seconds == 3600);
	http_request_clear(&arena, &req);

	str = "min-fresh=3600, no-transform";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 2);
	pair = (HTTPRequestCacheControlPair*)array_get(arr, 0);
	ASSERT(pair->name == HTTP_REQ_CC_MIN_FRESH);
	ASSERT(pair->seconds == 3600);
	pair = (HTTPRequestCacheControlPair*)array_get(arr, 1);
	ASSERT(pair->name == HTTP_REQ_CC_NO_TRANSFORM);
	ASSERT(pair->seconds == -1);
	http_request_clear(&arena, &req);

	str = "no-transform, min-fresh=3600";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(arr->size == 2);
	pair = (HTTPRequestCacheControlPair*)array_get(arr, 0);
	ASSERT(pair->name == HTTP_REQ_CC_NO_TRANSFORM);
	ASSERT(pair->seconds == -1);
	pair = (HTTPRequestCacheControlPair*)array_get(arr, 1);
	ASSERT(pair->name == HTTP_REQ_CC_MIN_FRESH);
	ASSERT(pair->seconds == 3600);
	http_request_clear(&arena, &req);

	str = "no-transform, min-fresh=3600a";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_BAD_CACHE_CONTROL);
	http_request_clear(&arena, &req);

	str = "no-transform=3600, min-fresh=3600";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_BAD_CACHE_CONTROL);
	http_request_clear(&arena, &req);

	str = "";
	res = http_process_cache_control_req(str, req);
	ASSERT(res == HTTP_BAD_CACHE_CONTROL);
	http_request_clear(&arena, &req);


}

TEST("http_process_user_agent_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	str = "Mozilla/5.0 (X11; Linux x86_64; rv:12.0) Gecko/20100101 Firefox/12.0";
	res = http_process_user_agent_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(strcmp(str, req->headers->user_agent) == 0);
	http_request_clear(&arena, &req);


}

TEST("http_process_date_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;

	HTTPDate* date = req->headers->date;

	str = "Tue, 29 Oct 2024 16:56:32 GMT";
	res = http_process_date_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(date->day_name == DAY_TUE);
	ASSERT(date->day == 29);
	ASSERT(date->month == MONTH_OCT);
	ASSERT(date->year == 2024);
	ASSERT(date->hour == 16);
	ASSERT(date->minute == 56);
	ASSERT(date->second == 32);
	http_request_clear(&arena, &req);

	str = "Thu, 05 Mar 2026 16:40:31 GMT";
	res = http_process_date_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(date->day_name == DAY_THU);
	ASSERT(date->day == 5);
	ASSERT(date->month == MONTH_MAR);
	ASSERT(date->year == 2026);
	ASSERT(date->hour == 16);
	ASSERT(date->minute == 40);
	ASSERT(date->second == 31);
	http_request_clear(&arena, &req);

	str = "Thu,";
	res = http_process_date_req(str, req);
	ASSERT(res == HTTP_BAD_DATE);
	http_request_clear(&arena, &req);



}

TEST("http_process_expect_req") {
	Arena arena = arena_init(HTTP_REQ_SIZE);
	HTTPRequest* req = http_request_init(&arena);
	int res;
	const char* str;



	str = "100-continue";
	res = http_process_expect_req(str, req);
	ASSERT(res == HTTP_SUCCESS);
	ASSERT(req->headers->expect == HTTP_EXP_100_CONTINUE);
	http_request_clear(&arena, &req);

	str = "100";
	res = http_process_expect_req(str, req);
	ASSERT(res == HTTP_BAD_EXPECT);
	http_request_clear(&arena, &req);

}

#endif
#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

static KORALL_HTTP_ROUTE(my_route) {

	char value[100] = { 0 };
	korall_request_param_get(req, "name", value, 99);

	korall_response_start_set(res, req, 200);
	korall_response_header_set(res, "Cache-Control", "no-store");
	korall_response_header_set(res, "Content-Type", "application/json");
	//korall_response_header_set(res, "Content-Length", "0");
	korall_response_body_set(res, "{\"res\":\"hello\"}");
}

int main(int argc, char* argv[]) {
	
	#if CMAKE

	// RESOURCES_PATH is from CMakeLists.txt
	korall_init(RESOURCES_PATH, stdout);

	#else

	// with Make
	korall_init("./resources/", NULL);

	#endif

	korall_http_routes_add("/", "GET", my_route);
	
	korall_run();

	return 0;
}

#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

#define USING_CMAKE true

static KORALL_HTTP_ROUTE(my_route) {

	char value[100] = { 0 };
	korall_request_param_get(req, "name", value, 99);

	korall_response_start_set(res, 200);
	korall_response_header_set(res, "Cache-Control", "no-store");
	korall_response_header_set(res, "Content-Type", "application/json");
	//korall_response_header_set(res, "Content-Length", "0");
	korall_response_body_set(res, "{\"res\":\"hello\"}");
}

int main(int argc, char* argv[]) {
	
	HTTPRoutes* routes = korall_http_routes_init();
	korall_http_routes_add(routes, "/", HTTP_GET, my_route);


	#if USING_CMAKE

	// RESOURCES_PATH is from CMakeLists.txt
	korall_run(RESOURCES_PATH, routes, NULL, NULL);

	#else

	// with Make
	korall_run("./resources/", routes, NULL, NULL);

	#endif

	return 0;
}

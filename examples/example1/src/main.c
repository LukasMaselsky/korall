#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

static KORALL_ROUTE(my_route) {

	char value[100] = { 0 };
	korall_request_param_get(req, "name", value, 99);

	korall_response_start_set(res, 200);
	korall_response_header_set(res, "Cache-Control", "no-store");
	korall_response_header_set(res, "Content-Type", "application/json");
	//korall_response_header_set(res, "Content-Length", "0");
	korall_response_body_set(res, "{\"res\":\"hello\"}");
}

int main(int argc, char* argv[]) {
	
	Routes* routes = korall_routes_init();
	korall_routes_add(routes, "/", HTTP_GET, my_route);

	// RESOURCES_PATH is from CMakeLists.txt
	korall_run(RESOURCES_PATH, routes);

	return 0;
}

#include "main.h"
#include "korall/server.h"
#include <stdio.h>

static void my_route(const HTTPRequest* req, HTTPResponse* res) {
	//res->start_line->status_code = HTTP_SC_401;
	printf("HELLO\n\n\n");
}

int main(int argc, char* argv[]) {

	const Routes* routes = http_routes_init();
	http_routes_add(routes, "/", HTTP_GET, my_route);
	
	// RESOURCES_PATH is from CMakeLists.txt
	http_server_run(RESOURCES_PATH, routes);

	return 0;
}

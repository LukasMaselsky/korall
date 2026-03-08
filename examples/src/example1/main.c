#include "main.h"
#include "korall/server.h"
#include <stdio.h>

static void my_route(const HTTPRequest* req, HTTPResponse* res) {
	//res->start_line->status_code = HTTP_SC_401;
	printf("HELLO\n\n\n");
}

int main(int argc, char* argv[]) {

	ServerConfig* config = http_config_init(NULL, "3500", "CustomServer");
	
	const Routes* routes = http_routes_init(1);
	http_routes_add(routes, "/", HTTP_GET, my_route);

	http_server_run(config, routes);

	return 0;
}

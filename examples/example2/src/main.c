#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

static KORALL_WS_ROUTE(my_route) {

	
}

int main(int argc, char* argv[]) {
	
	WebsocketRoutes* routes = korall_ws_routes_init();
	korall_ws_routes_add(routes, "/", my_route);

	// RESOURCES_PATH is from CMakeLists.txt
	korall_run(RESOURCES_PATH, NULL, routes);

	return 0;
}

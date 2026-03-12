#include "main.h"
#include "korall/korall.h"
#include <stdio.h>

static KORALL_ROUTE(my_route) {
	//res->start_line->status_code = HTTP_SC_401;
	//printf("HELLO\n\n\n");
}

int main(int argc, char* argv[]) {

	const Routes* routes = korall_routes_init();
	korall_routes_add(routes, "/", HTTP_GET, my_route);
	
	// RESOURCES_PATH is from CMakeLists.txt
	korall_run(RESOURCES_PATH, routes);

	return 0;
}

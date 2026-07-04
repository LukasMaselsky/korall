#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

#define USING_CMAKE true

static KORALL_WS_ROUTE(echo) {
	WebsocketFrame frame = { 0 };
	frame.data = data->data;
	frame.opcode = WS_OP_TEXT;
	frame.finished = true;
	frame.close_code = WS_CC_UNUSED;
	frame.length = data->length;
	frame.mask = false;
	frame.socket = data->socket;
	korall_ws_frame_send(&frame);
}

int main(int argc, char* argv[]) {

	WebsocketRoutes* routes = korall_ws_routes_init();
	korall_ws_routes_add(routes, "/", echo);

	FILE* log_file = NULL;

	#if USING_CMAKE
	
	log_file = fopen(RESOURCES_PATH "log_file.txt", "a");

	// RESOURCES_PATH is from CMakeLists.txt
	korall_run(RESOURCES_PATH, NULL, routes, log_file);
	
	#else

	log_file = fopen("./resources/log_file.txt", "a");

	// with Make
	korall_run("./resources/", NULL, routes, log_file);

	#endif



	return 0;
}

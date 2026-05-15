#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

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

	// RESOURCES_PATH is from CMakeLists.txt
	korall_run(RESOURCES_PATH, NULL, routes);

	return 0;
}

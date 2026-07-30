#include "main.h"
#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

#define USING_CMAKE true

static KORALL_WS_ROUTE(echo) {
	WebsocketFrame res_frame = { 0 };
	res_frame.data = frame->data;
	res_frame.opcode = WS_OP_TEXT;
	res_frame.finished = true;
	res_frame.close_code = WS_CC_UNUSED;
	res_frame.length = frame->length;
	res_frame.mask = false;
	res_frame.socket = frame->socket;
	res_frame.ssl = frame->ssl;
	korall_ws_frame_send(&res_frame);
}

int main(int argc, char* argv[]) {


	FILE* log_file = NULL;

	#if USING_CMAKE
	
	log_file = fopen(RESOURCES_PATH "log_file.txt", "a");

	// RESOURCES_PATH is from CMakeLists.txt
	korall_init(RESOURCES_PATH, log_file);
	
	#else

	log_file = fopen("./resources/log_file.txt", "a");

	// with Make
	korall_init("./resources/" log_file);

	#endif

	korall_ws_routes_add("/", echo);

	korall_run();

	fclose(log_file);

	return 0;
}

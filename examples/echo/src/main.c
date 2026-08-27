#include "korall/korall.h"
#include <stdio.h>
#include <string.h>

static KORALL_WS_ROUTE(echo) {
	
	res->data = req->data;
	res->opcode = WS_OP_TEXT;
	res->finished = true;
	res->close_code = WS_CC_UNUSED;
	res->length = req->length;
	res->mask = false;
	res->ssl = req->ssl;
	
}

int main() {

	FILE* log_file = NULL;

	#ifdef CMAKE
	
	log_file = fopen(RESOURCES_PATH "log_file.txt", "a");

	// RESOURCES_PATH is from CMakeLists.txt
	korall_init(RESOURCES_PATH, stdout);
	
	#else

	log_file = fopen("./resources/log_file.txt", "a");

	// with Make
	korall_init("./resources/", log_file);

	#endif

	korall_ws_routes_add("/", echo);

	korall_run();

	fclose(log_file);

	return 0;
}

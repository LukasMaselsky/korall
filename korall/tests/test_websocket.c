#if defined HEADERS
#include <string.h>
#include "websocket.h"
#elif defined TESTS

TEST("websocket_process_frame") {
	uint8_t data[100] = { 0 };
	data[0] = 128;
	websocket_process_frame(data);
	data[0] = 127;
	websocket_process_frame(data);
}

#endif
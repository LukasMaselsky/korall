#if defined HEADERS
#include <string.h>
#include "websocket.h"
#elif defined TESTS

// https://sachin9996.github.io/websocket-frames-explained/

TEST("websocket_process_frame") {
	uint8_t data[200] = { 0x81, 0x86, 0xe4, 0x99, 0xca, 0x9b, 0x8c, 0xfc, 0xa6, 0xf7, 0x8b, 0x93 };
	WebsocketFrame frame = { 0 };
	
	websocket_process_frame(data, &frame);
	ASSERT(frame.mask);
	ASSERT(frame.finished);
	ASSERT(frame.opcode == WS_OP_TEXT);
	ASSERT(frame.masking_key == 0x9bca99e4);
	ASSERT(frame.length == 6);
}

#endif
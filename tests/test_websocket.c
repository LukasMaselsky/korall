#if defined HEADERS
#include <string.h>
#include "http/websocket/websocket.h"
#elif defined TESTS

// https://sachin9996.github.io/websocket-frames-explained/

TEST("websocket_frame_decode")
{
	uint8_t data[200] = {0x81, 0x86, 0xe4, 0x99, 0xca, 0x9b, 0x8c, 0xfc, 0xa6, 0xf7, 0x8b, 0x93};
	WebsocketFrame frame = {0};

	websocket_frame_decode(data, &frame);
	ASSERT(frame.mask);
	ASSERT(frame.finished);
	ASSERT(frame.opcode == WS_OP_TEXT);
	ASSERT(frame.masking_key == 0x9bca99e4);
	ASSERT(frame.length == 6);
}

TEST("websocket_frame_encode")
{
	size_t size;
	uint8_t data[200] = {0};
	WebsocketFrame frame = {.finished = true, .opcode = WS_OP_TEXT, .mask = false, .length = 0, .data = NULL, .close_code = WS_CC_UNUSED};
	size = websocket_frame_encode(&frame, data);
	ASSERT(data[0] == 0x81);
	ASSERT(data[1] == 0);
	// websocket_frame_print(data, size);

	uint8_t data2[400] = {0};
	WebsocketFrame frame2 = {.finished = true, .opcode = WS_OP_TEXT, .mask = false, .length = 66528, .data = "aaa", .close_code = WS_CC_UNUSED};
	size = websocket_frame_encode(&frame2, data2);
	ASSERT(data2[0] == 0x81);
	ASSERT(data2[1] == 0x7f);
	ASSERT(data2[2] == 0);
	ASSERT(data2[3] == 0);
	ASSERT(data2[4] == 0);
	ASSERT(data2[5] == 0);
	ASSERT(data2[6] == 0);
	ASSERT(data2[7] == 0x01);
	ASSERT(data2[8] == 0x03);
	ASSERT(data2[9] == 0xe0);
	ASSERT(data2[10] == 0x61);
	// websocket_frame_print(data2, size);

	uint8_t data3[400] = {0};
	WebsocketFrame frame3 = {.finished = true, .opcode = WS_OP_TEXT, .mask = false, .length = 300, .data = "aaa", .close_code = WS_CC_UNUSED};
	size = websocket_frame_encode(&frame3, data3);
	ASSERT(data3[0] == 0x81);
	ASSERT(data3[1] == 0x7e);
	ASSERT(data3[2] == 0x01);
	ASSERT(data3[3] == 0x2c);
	ASSERT(data3[4] == 0x61);
	ASSERT(data3[5] == 0x61);
	ASSERT(data3[6] == 0x61);
	// websocket_frame_print(data3, size);

	uint8_t data4[400] = {0};
	WebsocketFrame frame4 = {0};
	websocket_frame_construct_close(&frame4, WS_CC_1000, false, 0);
	size = websocket_frame_encode(&frame4, data4);
	// websocket_frame_print(data4, size);

	uint8_t data5[400] = {0};
	WebsocketFrame frame5 = {.finished = true, .opcode = WS_OP_TEXT, .mask = false, .length = 3, .data = "hey", .close_code = WS_CC_UNUSED};
	size = websocket_frame_encode(&frame5, data5);
	ASSERT(data5[0] == 0x81);
	ASSERT(data5[1] == 0x03);
	ASSERT(data5[2] == 0x68);
	ASSERT(data5[3] == 0x65);
	ASSERT(data5[4] == 0x79);
}

#endif
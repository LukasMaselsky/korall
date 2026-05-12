#ifndef WEBSOCKET_H
#define WEBSOCKET_H

typedef enum {
	WS_OP_CON = 0,
	WS_OP_TEXT = 1,
	WS_OP_BIN = 2,
	WS_OP_CLOSE = 8,
	WS_OP_PING = 9,
	WS_OP_PONG = 10,
} WebsocketOpcode;

typedef enum {
	WS_CC_UNUSED = -1,
	WS_CC_1000 = 1000,
	WS_CC_1001 = 1001,
	WS_CC_1002 = 1002,
	WS_CC_1003 = 1003,
	WS_CC_1005 = 1005,
	WS_CC_1006 = 1006,
	WS_CC_1007 = 1007,
	WS_CC_1008 = 1008,
	WS_CC_1009 = 1009,
	WS_CC_1010 = 1010,
	WS_CC_1011 = 1011,
	WS_CC_1015 = 1015,
	WS_CC_COUNT = 12,
} WebsocketCloseCode;

typedef struct WebsocketFramePrivate WebsocketFrame;

int websocket_frame_send(const WebsocketFrame* frame);

#endif
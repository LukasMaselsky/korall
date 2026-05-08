#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "utils.h"


typedef enum {
	WS_OP_CON = 0,
	WS_OP_TEXT = 1,
	WS_OP_BIN = 2,
	WS_OP_CLOSE = 8,
	WS_OP_PING = 9,
	WS_OP_PONG = 10,
} WebsocketOpcode;


typedef struct {
	uint64_t length;
	uint32_t masking_key;
	WebsocketOpcode opcode;
	uint8_t* data;
	bool mask;
	bool finished;
} WebsocketFrame;

#define BITS_LAST(k,n) ((k) & ((1<<(n))-1))
#define BITS_MID(k,m,n) BITS_LAST((k)>>(m),((n)-(m)))

int websocket_process_frame(uint8_t* frame, WebsocketFrame* wsf);

#endif
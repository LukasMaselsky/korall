#ifndef WEBSOCKET_INTERNAL_H
#define WEBSOCKET_INTERNAL_H

#include "utils.h"
#include "korall/websocket.h"
#include "korall/korall.h";

#define WS_HEADER_SIZE ((3 * 32) + 16)
#define WS_FRAME_PAYLOAD_SIZE (1 * MEGABYTE) // change ?
#define WS_FRAME_SIZE (sizeof(WebsocketFrame) + WS_FRAME_PAYLOAD_SIZE)
#define WS_ARENA_SIZE (WS_FRAME_SIZE + KILOBYTE)
#define WS_FULL_ARENA_SIZE (WS_HEADER_SIZE + WS_FRAME_PAYLOAD_SIZE + KILOBYTE)


typedef struct {
	SOCKET socket;
	WebsocketRoute* route;
} WebsocketConnection;


#define BITS_LAST(k,n) ((k) & ((1<<(n))-1))
#define BITS_MID(k,m,n) BITS_LAST((k)>>(m),((n)-(m)))

int websocket_frame_decode(uint8_t* frame, WebsocketFrame* wsf);

int websocket_frame_construct(
	WebsocketFrame* wsf,
	SOCKET socket,
	bool finished,
	WebsocketOpcode opcode,
	WebsocketCloseCode close_code,
	bool mask,
	uint32_t masking_key,
	uint8_t* data
);

int websocket_frame_construct_close(
	WebsocketFrame* wsf,
	SOCKET socket,
	WebsocketCloseCode close_code,
	bool mask,
	uint32_t masking_key,
	uint8_t* data
);

int websocket_frame_construct_pong(
	WebsocketFrame* wsf,
	SOCKET socket,
	bool mask,
	uint32_t masking_key
);

int websocket_frame_encode(
	const WebsocketFrame* frame,
	uint8_t* data
);

#endif
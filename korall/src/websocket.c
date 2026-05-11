#include "websocket.h"
#include "utils.h"
#include "sockets.h"


int websocket_frame_process(uint8_t *frame, WebsocketFrame* wsf) {
	uint8_t b1 = frame[0];
	uint8_t b2 = frame[1];
	bool mask = BITS_MID(b2, 7, 8);

	if (!mask) return -1; // mask required on client -> server

	bool finished = BITS_MID(b1, 7, 8);

	uint8_t opcode = BITS_MID(b1, 0, 4);

	uint8_t pl = BITS_MID(b2, 0, 7);

	uint64_t length; // actual payload length
	uint32_t masking_key;
	uint32_t* mkeyp;
	
	if (pl >= 0 && pl <= 125) {
		length = (uint64_t)pl;
		mkeyp = (uint32_t*)(frame + 2);
	}
	else if (pl == 126) {
		length = (uint64_t)(*((uint16_t*)(frame + 2)));
		mkeyp = (uint32_t*)(frame + 4);
	}
	else {
		length = (uint64_t)(*((uint64_t*)(frame + 2)));
		mkeyp = (uint32_t*)(frame + 10);
	}
	masking_key = *mkeyp;
	uint8_t* data = (uint8_t*)(mkeyp + 1);
	uint8_t* m_key = (uint8_t*) &masking_key;

	// unmask data
	int i = 0;
	while (i < length) {
		data[i] = data[i] ^ m_key[i % 4]; // ! modifies wsf in place
		i++;
	}

	WebsocketCloseCode close_code = WS_CC_UNUSED;
	if (opcode == WS_OP_CLOSE) {
		close_code = data[0];
	}

	wsf->data = data;
	wsf->finished = finished;
	wsf->length = length;
	wsf->mask = mask;
	wsf->masking_key = masking_key;
	wsf->opcode = opcode;
	wsf->close_code = close_code;


	return 0;
}

int websocket_frame_construct(
	WebsocketFrame* wsf,
	SOCKET socket,
	bool finished,
	WebsocketOpcode opcode,
	WebsocketCloseCode close_code,
	bool mask,
	uint32_t masking_key,
	uint8_t* data
) {
	if (sizeof(data) + 1 > UINT64_MAX) {
		printf("Couldn't construct ws frame, data too long");
		return -1;
	};

	wsf->socket = socket;
	wsf->length = sizeof(data) + 1;
	wsf->finished = finished;
	wsf->opcode = opcode;
	wsf->close_code = close_code;
	wsf->data = data;
	wsf->mask = mask;
	wsf->masking_key = masking_key;

	return 0;
}

int websocket_frame_construct_pong(
	WebsocketFrame* wsf,
	SOCKET socket,
	bool mask,
	uint32_t masking_key
) {
	return websocket_frame_construct(wsf, socket, true, WS_OP_PONG, WS_CC_UNUSED, mask, masking_key, NULL);
}

int websocket_frame_construct_close(
	WebsocketFrame* wsf,
	SOCKET socket,
	WebsocketCloseCode close_code,
	bool mask,
	uint32_t masking_key,
	uint8_t* data
) {
	return websocket_frame_construct(wsf, socket, true, WS_OP_CLOSE, close_code, mask, masking_key, data);
}

int websocket_frame_send(const WebsocketFrame* frame, uint8_t* data) {
	// todo: convert

	// todo: move data arena create in here ?

	//sprintf(data, "%s%s%s", res->start_line.chars, res->headers_base, body);
	//printf("'%s'", data);
	
	if (data == NULL) {
		printf("server: failed to convert websocket data to str\n");
		return -1;
	}

	int r = socket_send(frame->socket, data, strlen(data), 0);
	if (r == -1) {
		printf("server: couldn't send data to ");
		socket_print(frame->socket);
		printf("\n");
	}
	
	return 0;
}
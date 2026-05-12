#include "websocket_internal.h"
#include "utils.h"
#include "sockets.h"
#include "arena.h"

/**
 * @brief converts raw frame to WebsocketFrame struct
 * @param frame
 * @param wsf
 * @return
 */
int websocket_frame_decode(uint8_t* frame, WebsocketFrame* wsf) {
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
	uint8_t* m_key = (uint8_t*)&masking_key;

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

/**
 * @brief converts WebsocketFrame struct to raw frame
 * @param frame
 * @param data
 * @return
 */
int websocket_frame_encode(const WebsocketFrame* frame, uint8_t* data) {
	uint8_t fin = (uint8_t)(frame->finished << 7);
	uint8_t opcode = (uint8_t)(frame->opcode);

	data[0] = fin | opcode;

	uint8_t mask = (uint8_t)(frame->mask << 7);
	uint8_t pl;
	uint8_t* p;
	if (frame->length <= 125) {
		pl = (uint8_t)(frame->length);
		p = data + 2;
	}
	else if (frame->length <= UINT16_MAX) {
		pl = 126;
		uint16_t len = (uint16_t)(frame->length);
		memcpy_reverse(data + 2, &len, sizeof(len)); // endian has to be reversed
		// todo: check system endianess ?
		p = data + 4;
	}
	else {
		pl = 127;
		uint64_t len = frame->length;
		memcpy_reverse(data + 2, &len, sizeof(len));
		p = data + 10;
	}
	data[1] = mask | pl;

	if (frame->mask) {
		memcpy(p, &(frame->masking_key), sizeof(frame->masking_key));
		p += sizeof(frame->masking_key);
	}

	if (frame->close_code != WS_CC_UNUSED) {
		memcpy(p, &(frame->close_code), 2);
		p += 2;
	}

	if (frame->length != 0) {
		memcpy(p, frame->data, sizeof(frame->data));
	}
	uint8_t t = data;
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
	if (sizeof(data) > UINT64_MAX) {
		printf("Couldn't construct ws frame, data too long");
		return -1;
	};

	wsf->socket = socket;
	wsf->length = data == NULL ? 0 : sizeof(data);
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

// PUBLIC FUNCTIONS

/**
 * @brief encodes and sends websocket frame
 * @param frame 
 * @return 
 */
int websocket_frame_send(const WebsocketFrame* frame) {

	Arena data_arena = arena_init(WS_FULL_ARENA_SIZE);
	uint8_t* data = (uint8_t*)arena_alloc(&data_arena, WS_FULL_ARENA_SIZE);

	websocket_frame_encode(frame, data);

	printf("server: sending frame\n");

	int r = socket_send(frame->socket, data, strlen(data), 0);
	if (r == -1) {
		printf("server: couldn't send data to ");
		socket_print(frame->socket);
		printf("\n");
	}
	arena_free(&data_arena);

	return 0;
}
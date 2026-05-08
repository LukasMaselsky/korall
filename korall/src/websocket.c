#include "websocket.h"
#include "utils.h"


int websocket_process_frame(uint8_t *frame, WebsocketFrame* wsf) {
	uint8_t b1 = frame[0];
	bool mask = BITS_MID(b1, 0, 1);

	if (!mask) return -1; // mask required on client -> server

	bool finished = BITS_MID(b1, 7, 8);

	uint8_t opcode = BITS_MID(b1, 1, 5);

	uint8_t pl = frame[1];

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

	wsf->data = data;
	wsf->finished = finished;
	wsf->length = length;
	wsf->mask = mask;
	wsf->masking_key = masking_key;
	wsf->opcode = opcode;	

	return 0;
}
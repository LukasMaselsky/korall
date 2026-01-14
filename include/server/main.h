#ifndef MAIN_H
#define MAIN_H

#include "utils.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

typedef enum {
	ST_TCP,
	ST_HTTP,
} ServerType;

typedef enum {
	F_BADFLAG = -1,
	F_TCP,
	F_HTTP,
} Flag;

typedef struct {
	ServerType servertype;
} Flags;


#endif
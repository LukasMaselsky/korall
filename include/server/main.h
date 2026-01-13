#ifndef MAIN_H
#define MAIN_H
#include "sockets.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

typedef struct { 
	char* key; 
	int val; 
} FlagLookupEntry;

typedef enum {
	F_BADFLAG = -1,
	F_TCP,
	F_HTTP,
} Flag;

typedef enum {
	ST_TCP,
	ST_HTTP,
} ServerType;

typedef struct {
	ServerType servertype;
} Flags;

Flags default_flags = {
	.servertype = ST_TCP,
};

FlagLookupEntry flag_lookup_table[] = {
	{"tcp", F_TCP},
	{"http", F_HTTP},
};

#define NUM_OF_FLAGS (sizeof(flag_lookup_table)/sizeof(flag_lookup_table[0]))


#endif
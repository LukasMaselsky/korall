#ifndef MAIN_H
#define MAIN_H
#include "sockets.h"

#define DEFAULT_SOCK_TYPE TCP
#define READ_BUFFER_LEN 1024

typedef struct { 
	char* key; 
	int val; 
} FlagLookupEntry;

typedef enum {
	F_BADFLAG = -1,
	F_TCP,
	F_UDP,
} Flag;

typedef struct {
	SocketType socktype;
} Flags;

FlagLookupEntry flag_lookup_table[] = {
	{"--tcp", F_TCP},
};

#define NUM_OF_FLAGS (sizeof(flag_lookup_table)/sizeof(flag_lookup_table[0]))


#endif
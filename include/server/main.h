#ifndef MAIN_H
#define MAIN_H
#include "sockets.h"

#define DEFAULT_SOCK_TYPE TCP

typedef struct { 
	char* key; 
	int val; 
} FlagLookupEntry;

typedef enum {
	F_BADFLAG = -1,
	F_TCP,
	F_UDP
} Flag;

typedef struct {
	SocketType socktype;
} Flags;

FlagLookupEntry flag_lookup_table[] = {
	{"--udp", F_UDP},
	{"--tcp", F_TCP},
};

#define NUM_OF_FLAGS (sizeof(flag_lookup_table)/sizeof(FlagLookupEntry))


#endif
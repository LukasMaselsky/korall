#ifndef MAIN_H
#define MAIN_H

#include "utils.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

#define DEFAULT_SERVER_NAME "MyServer"
#define MAX_SERVER_NAME_LEN 100 // ?

typedef enum {
	ST_TCP,
	ST_HTTP,
} ServerType;

typedef struct {
	ServerType type;
	char* domain;
	char* port;
	char* name;
} ServerConfig;

void http_server_run(ServerConfig* config);

#endif
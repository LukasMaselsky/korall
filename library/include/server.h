#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include "http_.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

#define DEFAULT_SERVER_NAME "MyServer"
#define MAX_SERVER_NAME_LEN 100 // todo 

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

typedef struct {
	const char* path;
	const HTTPMethod method;
	void (*const callback)(HTTPRequest*, HTTPResponse*);
} Route;

typedef struct {
	const Route* const routes;
	const size_t route_count;
} Routes;

void http_server_run(ServerConfig* config, const Routes *routes);

#endif
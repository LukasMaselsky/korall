#ifndef HTTP_SERVER_INTERNAL_H
#define HTTP_SERVER_INTERNAL_H

#include <stdlib.h>
#include "http_.h"
#include "http_server/http_server.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

#define DEFAULT_SERVER_NAME "MyServer"
#define MAX_SERVER_NAME_LEN 100 // todo 

enum ServerTypePrivate {
	ST_TCP,
	ST_HTTP,
};

struct ServerConfigPrivate {
	ServerType type;
	char* domain;
	char* port;
	char* name;
};

struct RoutePrivate {
	const char* path;
	const HTTPMethod method;
	void (* const callback)(HTTPRequest*, HTTPResponse*);
};

struct RoutesPrivate {
	const Route* const routes;
	const size_t route_count;
};

#endif
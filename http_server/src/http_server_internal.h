#ifndef HTTP_SERVER_INTERNAL_H
#define HTTP_SERVER_INTERNAL_H

#include <stdlib.h>
#include "http_internal.h"
#include "http_server/http_server.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

#define DEFAULT_SERVER_NAME "MyServer"
#define MAX_SERVER_NAME_LEN 100 // todo 


struct ServerConfigPrivate {
	char* domain;
	char* port;
	char* name;
};

struct RoutePrivate {
	const char* path;
	const HTTPMethod method;
	void (* const callback)(const HTTPRequest*, HTTPResponse*);
};

struct RoutesPrivate {
	Route* routes;
	size_t route_count;
	size_t capacity;
};

#endif
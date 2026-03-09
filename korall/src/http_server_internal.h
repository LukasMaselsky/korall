#ifndef HTTP_SERVER_INTERNAL_H
#define HTTP_SERVER_INTERNAL_H

#include <stdlib.h>
#include "http_internal.h"
#include "korall/server.h"

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

#define DEFAULT_SERVER_NAME "MyServer"
#define MAX_SERVER_NAME_LEN 100 // todo 

#define DEFAULT_PORT "3500"
#define DEFAULT_DOMAIN "localhost"

#define HTTP_CONFIG_BUFFER_LEN KILOBYTE * 10 // todo: ?

typedef struct ServerConfig {
	String domain;
	String port;
	String name;
} ServerConfig;

typedef enum HTTPConfigError {
	HTTP_CONF_ERROR = -1,
	HTTP_CONF_SUCCESS,
	HTTP_CONF_DEFAULT,
} HTTPConfigError;

HTTPConfigError http_config_init(ServerConfig* config);


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
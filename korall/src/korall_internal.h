#ifndef KORALL_INTERNAL_H
#define KORALL_INTERNAL_H

#include <stdlib.h>
#include "http_internal.h"
#include "websocket_internal.h"
#include "korall/korall.h"

#define READ_BUFFER_LEN KILOBYTE
#define RESPONSE_BUFFER_LEN KILOBYTE

#define DEFAULT_SERVER_NAME "KorallServer"
#define MAX_SERVER_NAME_LEN 100 // todo 
#define SERVER_CONFIG_FILE_NAME "korall_config.json"
#define DEFAULT_PORT "3500"
#define DEFAULT_DOMAIN "localhost"
#define SERVER_SOFTWARE "Korall"

#define HTTP_CONFIG_BUFFER_LEN KILOBYTE * 10 // todo: ?
#define HTTP_ROUTES_CAPACITY 100 // todo: ?
#define WS_ROUTES_CAPACITY 100 // todo: ?


int http_config_init(const char *path, ServerConfig* config, ServerConfig *default_config);


struct HTTPRoutePrivate {
	const char* path;
	const HTTPMethod method;
	void (* const callback)(const HTTPRequest*, HTTPResponse*);
};

struct HTTPRoutesPrivate {
	HTTPRoute* routes;
	size_t route_count;
	size_t capacity;
};

struct WebsocketRoutePrivate {
	const char* path;
	void (* const callback)(const WebsocketFrame*);
};

struct WebsocketRoutesPrivate {
	WebsocketRoute* routes;
	size_t route_count;
	size_t capacity;
};

#endif
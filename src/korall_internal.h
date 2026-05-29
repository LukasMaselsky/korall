#ifndef KORALL_INTERNAL_H
#define KORALL_INTERNAL_H

#include <stdlib.h>
#include "http_internal.h"
#include "websocket_internal.h"
#include "array.h"
#include "korall/korall.h"

#define READ_BUFFER_LEN KILOBYTE
#define RESPONSE_BUFFER_LEN KILOBYTE

#define HTTP_ROUTES_CAPACITY 100 // todo: ?
#define WS_ROUTES_CAPACITY 100 // todo: ?

#define MAX_THREADS 32


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

typedef struct {
	SOCKET sock;
	HTTPRoutes* http_routes;
	WebsocketRoutes* ws_routes;
} ProcessArgs;

#endif
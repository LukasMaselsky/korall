#ifndef KORALL_INTERNAL_H
#define KORALL_INTERNAL_H

#include <stdlib.h>
#include "http/http_internal.h"
#include "http/websocket/websocket_internal.h"
#include "array/array.h"
#include "korall/korall.h"
#include "thread/thread.h"

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
	size_t thread_num;
	pthread_mutex_t *lock;
	Array* thread_arr;
	SOCKET sock;
	HTTPRoutes* http_routes;
	WebsocketRoutes* ws_routes;
} ProcessDataArgs;

#endif
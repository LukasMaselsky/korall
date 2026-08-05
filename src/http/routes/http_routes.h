#ifndef KORALL_HTTP_ROUTES_H
#define KORALL_HTTP_ROUTES_H

#include "utils/utils.h"
#include "config/config.h"
#include "korall/websocket.h"
#include "http/http_internal.h"
#include "array/array.h"

typedef struct
{
	const char *path;
	void (*const callback)(const WebsocketFrame *);
} WebsocketRoute;

typedef struct
{
	const char *path;
	const HTTPMethod method;
	void (*const callback)(const HTTPRequest *, HTTPResponse *);
} HTTPRoute;


Array *http_routes_get();

Array *ws_routes_get();

void http_routes_init(ServerConfig *config);

void ws_routes_init(ServerConfig *config);

void routes_init(ServerConfig *config);

void routes_free(Array *http, Array *ws);

#endif
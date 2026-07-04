#ifndef KORALL_H
#define KORALL_H

#include <stdlib.h>
#include <stdio.h>
#include "http.h"
#include "websocket.h"

#define KORALL_HTTP_ROUTE(name) void name(const HTTPRequest* req, HTTPResponse* res)
#define KORALL_WS_ROUTE(name) void name(const WebsocketFrame* frame)

typedef struct HTTPRoutePrivate HTTPRoute;

typedef struct HTTPRoutesPrivate HTTPRoutes;

typedef struct WebsocketRoutePrivate WebsocketRoute;

typedef struct WebsocketRoutesPrivate WebsocketRoutes;

void korall_run(const char *config_path, const HTTPRoutes *http_routes, const WebsocketRoutes* ws_routes, const FILE* log_file);

HTTPRoutes* korall_http_routes_init();

WebsocketRoutes* korall_ws_routes_init();

void korall_http_routes_add(HTTPRoutes* routes, const char* path, const HTTPMethod method, void (* const callback)(const HTTPRequest*, HTTPResponse*));

void korall_ws_routes_add(WebsocketRoutes* routes, const char* path, void (* const callback)(const WebsocketFrame*));


#endif
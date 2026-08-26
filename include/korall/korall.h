#ifndef KORALL_H
#define KORALL_H

#include <stdlib.h>
#include <stdio.h>
#include "http.h"
#include "websocket.h"

#define KORALL_HTTP_ROUTE(name) void name(const HTTPRequest* req, HTTPResponse* res)
#define KORALL_WS_ROUTE(name) void name(const WebsocketFrame* req, WebsocketFrame* res)

typedef void (*KorallHTTPRoute)(const HTTPRequest* req, HTTPResponse* res);
typedef void (*KorallWSRoute)(const WebsocketFrame* req, WebsocketFrame* res);


void korall_init(const char* config_path, const FILE* log_file);

void korall_run();

void korall_http_routes_add(const char* path, const char* method, KorallHTTPRoute callback);

void korall_ws_routes_add(const char* path, KorallWSRoute callback);


#endif
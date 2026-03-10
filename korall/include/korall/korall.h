#ifndef KORALL_H
#define KORALL_H

#include <stdlib.h>
#include "http.h"

#define KORALL_ROUTE(name) void name##(const HTTPRequest* req, HTTPResponse* res)



typedef struct RoutePrivate Route;

typedef struct RoutesPrivate Routes;

void korall_run(const char *config_path, const Routes *routes);

Routes* korall_routes_init();

void korall_routes_add(Routes* routes, const char* path, const HTTPMethod method, void (* const callback)(const HTTPRequest*, HTTPResponse*));


#endif
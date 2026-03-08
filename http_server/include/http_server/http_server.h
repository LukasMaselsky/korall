#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdlib.h>
#include "http.h"

typedef struct ServerConfigPrivate ServerConfig;

typedef struct RoutePrivate Route;

typedef struct RoutesPrivate Routes;

void http_server_run(ServerConfig* config, const Routes *routes);

ServerConfig* http_config_init(const char* domain, const char* port, const char* name);

Routes* http_routes_init(const size_t capacity);

Routes* http_routes_add(Routes* routes, const char* path, const HTTPMethod method, void (* const callback)(const HTTPRequest*, HTTPResponse*));


#endif
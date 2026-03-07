#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdlib.h>

#define READ_BUFFER_LEN 1024
#define RESPONSE_BUFFER_LEN 1024

#define DEFAULT_SERVER_NAME "MyServer"
#define MAX_SERVER_NAME_LEN 100 // todo 

typedef enum ServerTypePrivate ServerType;

typedef struct ServerConfigPrivate ServerConfig;

typedef struct RoutePrivate Route;

typedef struct RoutesPrivate Routes;

void http_server_run(ServerConfig* config, const Routes *routes);

#endif
#ifndef KORALL_CONFIG_H
#define KORALL_CONFIG_H

#include "utils/utils.h"
#include <cjson/cJSON.h>
#include "socket/socket.h"
#include "array/array.h"

#define DEFAULT_PORT "3500"
#define DEFAULT_DOMAIN "localhost"
#define SERVER_SOFTWARE "Korall"
#define SERVER_CONFIG_FILE_NAME "korall_config.json"
#define DEFAULT_SERVER_NAME "KorallServer"
#define MAX_ALLOW_ORIGINS 100 // todo
#define MAX_ALLOW_METHODS HTTP_METHOD_COUNT
#define MAX_ALLOW_HEADERS 100 // todo
#define MAX_SERVER_NAME_LEN 100 // todo 
#define CONFIG_BUFFER_LEN KILOBYTE * 10 // todo: ?
#define ANY_ALLOW_METHODS HTTP_METHOD_COUNT + 1
#define ALL_METHODS_LIST_STR_LEN 52 // len "CONNECT,DELETE,GET,HEAD,OPTIONS,PATCH,POST,PUT,TRACE"

typedef struct ServerConfig {
	const char* resource_path;
	String domain;
	String port;
	String name;
	Array* allow_origins; // char **
	Array* allow_headers; // char **
	Array* allow_methods; // int
	bool allow_credentials;
	bool secure;
	unsigned int max_http_routes;
	unsigned int max_ws_routes;
	bool on_heap;
} ServerConfig;

ServerConfig* config_get();

void config_free(ServerConfig* config);

ServerConfig* config_init(const char* path);

#endif
#ifndef CONFIG_H
#define CONFIG_H

#include "utils/utils.h"
#include "cJSON/cJSON.h"
#include "socket/socket.h"
#include "array/array.h"

#define DEFAULT_PORT "3500"
#define DEFAULT_DOMAIN "localhost"
#define SERVER_SOFTWARE "Korall"
#define SERVER_CONFIG_FILE_NAME "korall_config.json"
#define DEFAULT_SERVER_NAME "KorallServer"
#define MAX_ALLOW_ORIGINS 100 // todo
#define MAX_SERVER_NAME_LEN 100 // todo 
#define CONFIG_BUFFER_LEN KILOBYTE * 10 // todo: ?

typedef struct ServerConfig {
	String domain;
	String port;
	String name;
	Array* allow_origins;
	unsigned int max_http_routes;
	unsigned int max_ws_routes;
	bool allow_custom_headers;
	bool on_heap;
} ServerConfig;

ServerConfig* config_get();

void config_free(ServerConfig* config);

ServerConfig* config_init(const char* path);

#endif
#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include "cJSON.h"
#include "sockets.h"

#define DEFAULT_PORT "3500"
#define DEFAULT_DOMAIN "localhost"
#define SERVER_SOFTWARE "Korall"
#define SERVER_CONFIG_FILE_NAME "korall_config.json"
#define DEFAULT_SERVER_NAME "KorallServer"
#define MAX_SERVER_NAME_LEN 100 // todo 
#define CONFIG_BUFFER_LEN KILOBYTE * 10 // todo: ?

typedef struct ServerConfig {
	String domain;
	String port;
	String name;
	bool allow_custom_headers;
} ServerConfig;

void config_free(ServerConfig* config);

ServerConfig* config_init(const char* path);

extern ServerConfig g_default_config;
extern ServerConfig g_config;

#endif
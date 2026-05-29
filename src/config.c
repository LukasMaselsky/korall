#include "http_internal.h"
#include "cJSON.h"

ServerConfig g_default_config = {
	.domain = {.chars = DEFAULT_DOMAIN, .size = 9 },
	.port = {.chars = DEFAULT_PORT, .size = 4 },
	.name = {.chars = DEFAULT_SERVER_NAME, .size = 12 },
	.allow_custom_headers = true,
	.on_heap = false
};

ServerConfig g_config = { 0 };

void config_free(ServerConfig* config) {
	if (config == NULL || !(config->on_heap)) return;

	free(config->domain.chars);
	free(config->port.chars);
	free(config->name.chars);
}

static ServerConfig* config_alloc(ServerConfig* config) {

	config->domain.chars = safe_calloc(1, MAX_DOMAIN_LEN);
	config->domain.size = MAX_DOMAIN_LEN;

	config->port.chars = safe_calloc(1, MAX_PORT_NUM_CHAR_LEN);
	config->port.size = MAX_PORT_NUM_CHAR_LEN;

	config->name.chars = safe_calloc(1, MAX_SERVER_NAME_LEN);
	config->name.size = MAX_SERVER_NAME_LEN;

	config->on_heap = true;

	return config;
}

static void cjson_read_string(cJSON* json, String str, String def, const char* field) {
	cJSON* item = cJSON_GetObjectItemCaseSensitive(json, field);
	const char* val;
	if (!(cJSON_IsString(item) && (item->valuestring != NULL))) {
		printf("Config \"%s\" field is not valid, using default value.\n", field);
		val = def.chars;
	}
	else {
		if (strlen(item->valuestring) > str.size) {
			printf("Config \"%s\" field is too long, maximum %zu characters, using default value.\n", field, str.size);
			val = def.chars;
		}
		else {
			val = item->valuestring;
		}
	}
	strncpy(str.chars, val, str.size);
	return;
}

static void cjson_read_bool(cJSON* json, bool* val, bool def, const char* field) {
	cJSON* item = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsBool(item))) {
		printf("Config \"%s\" field is not valid, using default value.\n", field);
		*val = def;
	}
	else {
		*val = item->valueint;
	}
}

/**
 * @brief 
 * @param path 
 * @return -1 if use default, 0 if use custom 
 */
static int config_init_inner(const char* path) {

	ServerConfig* config = config_alloc(&g_config);
	ServerConfig* default_config = &g_default_config;

	const char* config_file_name = SERVER_CONFIG_FILE_NAME;
	char file_path[MAX_FILE_PATH + 1] = { 0 };

	if (path == NULL) {
		printf("Could not find a korall_config.json, using default config. If you are using a custom config, make sure the path is correct.\n");
		return -1;
	}
	else {
		size_t path_len = strlen(path);
		if (path_len > MAX_FILE_PATH) {
			printf("File path too long.");
			return -1;
		};
		strcpy(file_path, path);
		if (strlen(config_file_name) + path_len > MAX_FILE_PATH) {
			printf("File path too long.");
			return -1;
		};
		strcat(file_path, config_file_name);
	}

	FILE* fp = fopen(file_path, "r");
	if (fp == NULL) {
		printf("Could not find a korall_config.json, using default config. If you are using a custom config, make sure the path is correct.\n");
		return -1;
	};

	// read the file contents into a string
	char buffer[CONFIG_BUFFER_LEN + 1];
	fread(buffer, 1, sizeof(buffer), fp);
	if (ferror(fp)) {
		printf("Could not read from korall_config.json, using default config.\n");
		return -1;
	}
	fclose(fp);

	// parse the JSON data
	cJSON* json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("%s\n", error_ptr);
		}
		cJSON_Delete(json);
		return -1;
	}

	// access the JSON data

	// go through all ServerConfig

	// server name

	cjson_read_string(json, config->name, default_config->name, "name");

	// domain

	cjson_read_string(json, config->domain, default_config->domain, "domain");

	// port

	cJSON* port = cJSON_GetObjectItemCaseSensitive(json, "port");
	if (!(cJSON_IsNumber(port))) {
		printf("Config \"port\" field is not valid.\n");
		strncpy(config->port.chars, default_config->port.chars, config->port.size);
	}
	else if (!is_valid_port_num(port->valueint)) {
		printf("Config \"port\" field number is not valid, must be between %d and %d.\n", MIN_PORT_NUM, MAX_PORT_NUM);
		strncpy(config->port.chars, default_config->port.chars, config->port.size);
	}
	else {
		int_to_str(port->valueint, config->port.chars);
	};


	// allow_custom_headers

	cjson_read_bool(json, &(config->allow_custom_headers), default_config->allow_custom_headers, "allow_custom_headers");

	cJSON_Delete(json);

	return 0;
}

/**
 * @brief Loads config from .json file, else fills config with default values from default_config
 * @param path path of configuration file location
 * @return
 */
void config_init(const char* path) {

	int res = config_init_inner(path);
	if (res == -1) {
		g_config = g_default_config;
	}
	return;
}

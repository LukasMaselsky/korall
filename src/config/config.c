#include "http/http.h"
#include "server/http/server_http.h"
#include "lookup/lookup_tables.h"
#include "utils/utils.h"
#include <stdarg.h>
#include <stdbool.h>

static char* g_default_any = "*";

static ServerConfig g_config = { 0 };

ServerConfig *config_get()
{
	return &g_config;
}

void config_free(ServerConfig *config)
{
	if (config == NULL)
		return;

	free(config->resource_path);
	free(config->domain);
	free(config->port);
	free(config->name);
	free(config->allow_origins);
	free(config->allow_headers);
	free(config->allow_methods);
}

static ServerConfig *config_alloc(ServerConfig *config)
{
	config->resource_path = exit_calloc(1, MAX_FILE_PATH);
	config->domain = exit_calloc(1, MAX_DOMAIN_LEN);
	config->port = exit_calloc(1, MAX_PORT_NUM_CHAR_LEN);
	config->name = exit_calloc(1, MAX_SERVER_NAME_LEN);

	Array *arr = (Array *)array_create_heap(sizeof(char *), MAX_ALLOW_ORIGINS);
	if (arr == NULL)
		exit(EXIT_FAILURE);
	config->allow_origins = arr;

	Array *arr2 = (Array *)array_create_heap(sizeof(char *), MAX_ALLOW_HEADERS);
	if (arr == NULL)
		exit(EXIT_FAILURE);
	config->allow_headers = arr2;

	Array *arr3 = (Array *)array_create_heap(sizeof(int), MAX_ALLOW_METHODS);
	if (arr == NULL)
		exit(EXIT_FAILURE);
	config->allow_methods = arr3;

	return config;
}

static ServerConfig* config_init_default(ServerConfig *config) {
	
	strcpy(config->domain, DEFAULT_DOMAIN);
	strcpy(config->port, DEFAULT_PORT);
	strcpy(config->name, DEFAULT_SERVER_NAME);
	array_push(config->allow_origins, &g_default_any);
	array_push(config->allow_headers, &g_default_any);
	const int any_allow_methods = ANY_ALLOW_METHODS;
	array_push(config->allow_methods, &any_allow_methods);
	config->allow_credentials = false;
	config->secure = true;
	config->max_http_routes = HTTP_ROUTES_CAPACITY;
	config->max_ws_routes = WS_ROUTES_CAPACITY;
	return config;
}

static inline void log_field_not_valid(const char *field)
{
	KORALL_LOG(LOG_WARN, "Config \"%s\" field is not valid, using default value.\n", field);
}

static void cjson_read_string(cJSON *json, char *str, const char *field, size_t max_len)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field);

	if (!(cJSON_IsString(item) && (item->valuestring != NULL)))
	{
		log_field_not_valid(field);
		return;
	}
	
	if (strlen(item->valuestring) > max_len)
	{
		KORALL_LOG(LOG_WARN, "Config \"%s\" field is too long, maximum %zu characters, using default value.\n", field, max_len);
		return;
	}
	
	strncpy(str, item->valuestring, max_len);
	return;
}

static void cjson_read_bool(cJSON *json, bool* out, const char *field)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsBool(item)))
	{
		log_field_not_valid(field);
		return;
	}
	memcpy(out, &(item->valueint), sizeof(bool));
}

static void cjson_read_num(cJSON *json, int* out, const char *field)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsNumber(item)))
	{
		log_field_not_valid(field);
		return;
	}
	memcpy(out, &(item->valueint), sizeof(int));
}

static void cjson_read_arr_string(cJSON *json, const char *field, void (*callback)(const char *, Array*), Array* array)
{
	cJSON *arr = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsArray(arr)))
	{
		log_field_not_valid(field);
		return;
	};

	bool use_default = true;
	const cJSON *item = NULL;
	cJSON_ArrayForEach(item, arr)
	{
		if (!cJSON_IsString(item) || item->valuestring == NULL)
			continue;

		if (use_default) {
			array_clear(array);
			use_default = false;
		}

		char *str = item->valuestring;
		callback(str, array);
	}

	return;
}

static void allow_origins_add(const char *str, Array* origins_arr)
{
	const size_t str_len = strlen(str);
	// todo: MAX_HTTP_URL ?
	if (str_len > MAX_DOMAIN_LEN)
	{
		KORALL_LOG(LOG_WARN, "failed to load origin from \"allow_origins\", max size is %d\n", MAX_DOMAIN_LEN);
		return;
	}
	char *origin = exit_calloc(1, str_len + 1);
	strncpy(origin, str, str_len);
	int res = array_push(origins_arr, &origin);
	if (res == -1)
	{
		KORALL_LOG(LOG_WARN, "failed to load origin from \"allow_origins\", over capacity\n");
	}
}

static void allow_headers_add(const char *str, Array* arr)
{
	const size_t str_len = strlen(str);
	if (str_len > MAX_HTTP_HEADER_FIELD_LEN)
	{
		KORALL_LOG(LOG_WARN, "failed to load header from \"allow_headers\", max size is %d\n", MAX_HTTP_HEADER_FIELD_LEN);
		return;
	}
	char *header = exit_calloc(1, str_len + 1);
	strncpy(header, str, str_len);
	int res = array_push(arr, &header);
	if (res == -1)
	{
		KORALL_LOG(LOG_WARN, "failed to load header from \"allow_headers\", over capacity\n");
	}
}

static bool comp_int(const void *a, const void *b)
{
	const int a_i = *((int *)a);
	const int b_i = *((int *)b);
	return a == b;
}

static void allow_methods_add(const char *str, Array* arr)
{
	HTTPMethod method = lookup_str_int(str, &http_method_lookup_table, false);
	if (method == -1)
	{
		KORALL_LOG(LOG_WARN, "failed to load method from \"allow_methods\", invalid method value \"%s\"\n", str);
		return;
	}

	// check if duplicate

	if (array_find(arr, &method, comp_int) != -1)
		return; // duplicate

	int res = array_push(arr, &method);
	if (res == -1)
	{
		KORALL_LOG(LOG_WARN, "failed to load method from \"allow_methods\", over capacity\n");
	}
}

static void config_init_inner(const char *path)
{

	ServerConfig *config = config_alloc(&g_config);
	config_init_default(config);

	const char *config_file_name = SERVER_CONFIG_FILE_NAME;
	char file_path[MAX_FILE_PATH + 1] = {0};

	if (path == NULL)
	{
		KORALL_LOG(LOG_WARN, "Could not find a korall_config.json, using default config. If you are using a custom config, make sure the path is correct.\n");
		free(config->resource_path);
		config->resource_path = NULL;
		return;
	}
	
	if (str_concat(path, config_file_name, file_path, MAX_FILE_PATH) != 0)
	{
		KORALL_LOG(LOG_WARN, "File path too long, using default config.");
		free(config->resource_path);
		config->resource_path = NULL;
		return;
	}
	

	FILE *fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		KORALL_LOG(LOG_WARN, "Could not find a korall_config.json, using default config. If you are using a custom config, make sure the path is correct.\n");
		free(config->resource_path);
		config->resource_path = NULL;
		return;
	};

	// read the file contents into a string
	char buffer[CONFIG_BUFFER_LEN + 1];
	fread(buffer, 1, sizeof(buffer), fp);
	if (ferror(fp))
	{
		KORALL_LOG(LOG_WARN, "Could not read from korall_config.json, using default config.\n");
		free(config->resource_path);
		config->resource_path = NULL;
		return;
	}
	fclose(fp);

	// parse the JSON data
	cJSON *json = cJSON_Parse(buffer);
	if (json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			KORALL_LOG(LOG_ERR, "%s\n", error_ptr);
		}
		cJSON_Delete(json);
		return;
	}

	// access the JSON data

	// go through all ServerConfig

	// resource path

	strcpy(config->resource_path, path);

	// server name

	cjson_read_string(json, config->name, "name", MAX_SERVER_NAME_LEN);

	// domain

	cjson_read_string(json, config->domain, "domain", MAX_DOMAIN_LEN);

	// port

	int port = 0;
	str_to_int(&port, config->port, 10);
	cjson_read_num(json, &port, "port");

	if (!is_valid_port_num(port))
	{
		KORALL_LOG(LOG_WARN, "Config \"port\" field number is not valid, must be between %d and %d.\n", MIN_PORT_NUM, MAX_PORT_NUM);
	}
	else
	{
		int_to_str(port, config->port);
	};

	// max_http_routes

	cjson_read_num(json, &(config->max_http_routes), "max_http_routes");

	// max_ws_routes

	cjson_read_num(json, &(config->max_ws_routes), "max_ws_routes");

	// allow_credentials

	cjson_read_bool(json, &(config->allow_credentials), "allow_credentials");

	// allow_origins

	cjson_read_arr_string(json, "allow_origins", allow_origins_add, config->allow_origins);

	// allow_headers

	cjson_read_arr_string(json, "allow_headers", allow_headers_add, config->allow_headers);

	// allow_methods

	cjson_read_arr_string(json, "allow_methods", allow_methods_add, config->allow_methods);
	
	// secure

	cjson_read_bool(json, &(config->secure), "secure");

	cJSON_Delete(json);
	return;
}

/**
 * @brief Loads config from .json file, else fills config with default values from default_config
 * @param path path of configuration file location
 * @return
 */
ServerConfig *config_init(const char *path)
{
	config_init_inner(path);
	return &g_config;
}

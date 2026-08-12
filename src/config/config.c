#include "http/http.h"
#include "server/http/server_http.h"
#include "lookup/lookup_tables.h"
#include <stdarg.h>

static char *g_default_allow[1] = {"*"};
static Array g_default_allow_arr = array_create_stack(&g_default_allow, sizeof(char *), 1, 1);

// made int Array to easily remove duplicates when loading
static int g_default_allow_methods[1] = {ANY_ALLOW_METHODS}; // represents "*"
static Array g_default_allow_methods_arr = array_create_stack(&g_default_allow_methods, sizeof(int), 1, 1);

static ServerConfig g_default_config = {
	.resource_path = NULL,
	.domain = {.chars = DEFAULT_DOMAIN, .size = sizeof(DEFAULT_DOMAIN) - 1},
	.port = {.chars = DEFAULT_PORT, .size = sizeof(DEFAULT_PORT) - 1},
	.name = {.chars = DEFAULT_SERVER_NAME, .size = sizeof(DEFAULT_SERVER_NAME) - 1},
	.allow_origins = &g_default_allow_arr,
	.allow_headers = &g_default_allow_arr,
	.allow_methods = &g_default_allow_methods_arr,
	.allow_credentials = false,
	.secure = true,
	.max_http_routes = HTTP_ROUTES_CAPACITY,
	.max_ws_routes = WS_ROUTES_CAPACITY,
	.on_heap = false};

static ServerConfig g_config = {0};

ServerConfig *config_get()
{
	return &g_config;
}

void config_free(ServerConfig *config)
{
	if (config == NULL || !(config->on_heap))
		return;

	free(config->domain.chars);
	free(config->port.chars);
	free(config->name.chars);
	// ! todo: could be on stack (UNDEFINED, FIX) ?
	// ! todo: FIX
	free(config->allow_origins);
	free(config->allow_headers);
	free(config->allow_methods);
}

static ServerConfig *config_alloc(ServerConfig *config)
{

	config->domain.chars = exit_calloc(1, MAX_DOMAIN_LEN);
	config->domain.size = MAX_DOMAIN_LEN;

	config->port.chars = exit_calloc(1, MAX_PORT_NUM_CHAR_LEN);
	config->port.size = MAX_PORT_NUM_CHAR_LEN;

	config->name.chars = exit_calloc(1, MAX_SERVER_NAME_LEN);
	config->name.size = MAX_SERVER_NAME_LEN;

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

	config->on_heap = true;

	return config;
}

static inline void log_field_not_valid(const char *field)
{
	KORALL_LOG(LOG_WARN, "Config \"%s\" field is not valid, using default value.\n", field);
}

static void cjson_read_string(cJSON *json, String str, String default_val, const char *field)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field);
	const char *val;
	if (!(cJSON_IsString(item) && (item->valuestring != NULL)))
	{
		log_field_not_valid(field);
		val = default_val.chars;
	}
	else
	{
		if (strlen(item->valuestring) > str.size)
		{
			KORALL_LOG(LOG_WARN, "Config \"%s\" field is too long, maximum %zu characters, using default value.\n", field, str.size);
			val = default_val.chars;
		}
		else
		{
			val = item->valuestring;
		}
	}
	strncpy(str.chars, val, str.size);
	return;
}

static bool cjson_read_bool(cJSON *json, bool default_val, const char *field)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsBool(item)))
	{
		log_field_not_valid(field);
		return default_val;
	}
	return item->valueint;
}

static int cjson_read_num(cJSON *json, int default_val, const char *field)
{
	cJSON *item = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsNumber(item)))
	{
		log_field_not_valid(field);
		return default_val;
	}
	return item->valueint;
}

static int cjson_read_arr_string(cJSON *json, const char *field, void (*callback)(const char *, va_list), ...)
{
	cJSON *arr = cJSON_GetObjectItemCaseSensitive(json, field);
	if (!(cJSON_IsArray(arr)))
	{
		log_field_not_valid(field);
		return -1;
	};

	const cJSON *item = NULL;
	cJSON_ArrayForEach(item, arr)
	{
		if (!cJSON_IsString(item))
			continue;
		char *str = item->valuestring;
		va_list argp;
		va_start(argp, callback);
		callback(str, argp);
		va_end(argp);
	}

	return 0;
}

static void allow_origins_add(const char *str, va_list args)
{
	Array *origins_arr = va_arg(args, Array *);
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

static void allow_headers_add(const char *str, va_list args)
{
	Array *arr = va_arg(args, Array *);
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

static void allow_methods_add(const char *str, va_list args)
{
	Array *arr = va_arg(args, Array *);
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

/**
 * @brief
 * @param path
 * @return -1 if use default, 0 if use custom
 */
static int config_init_inner(const char *path)
{

	ServerConfig *config = config_alloc(&g_config);
	ServerConfig *default_config = &g_default_config;

	const char *config_file_name = SERVER_CONFIG_FILE_NAME;
	char file_path[MAX_FILE_PATH + 1] = {0};

	if (path == NULL)
	{
		KORALL_LOG(LOG_WARN, "Could not find a korall_config.json, using default config. If you are using a custom config, make sure the path is correct.\n");
		return -1;
	}
	else
	{
		if (str_concat(path, config_file_name, file_path, MAX_FILE_PATH) != 0)
		{
			KORALL_LOG(LOG_WARN, "File path too long, using default config.");
			return -1;
		}
	}

	FILE *fp = fopen(file_path, "r");
	if (fp == NULL)
	{
		KORALL_LOG(LOG_WARN, "Could not find a korall_config.json, using default config. If you are using a custom config, make sure the path is correct.\n");
		return -1;
	};

	// read the file contents into a string
	char buffer[CONFIG_BUFFER_LEN + 1];
	fread(buffer, 1, sizeof(buffer), fp);
	if (ferror(fp))
	{
		KORALL_LOG(LOG_WARN, "Could not read from korall_config.json, using default config.\n");
		return -1;
	}
	fclose(fp);

	// parse the JSON data
	cJSON *json = cJSON_Parse(buffer);
	if (json == NULL)
	{
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL)
		{
			printf("%s\n", error_ptr);
		}
		cJSON_Delete(json);
		return -1;
	}

	// access the JSON data

	// go through all ServerConfig

	// resource path

	config->resource_path = path;

	// server name

	cjson_read_string(json, config->name, default_config->name, "name");

	// domain

	cjson_read_string(json, config->domain, default_config->domain, "domain");

	// port

	int def;
	str_to_int(&def, default_config->port.chars, 10);
	int port = cjson_read_num(json, def, "port");

	if (!is_valid_port_num(port))
	{
		KORALL_LOG(LOG_WARN, "Config \"port\" field number is not valid, must be between %d and %d.\n", MIN_PORT_NUM, MAX_PORT_NUM);
		strncpy(config->port.chars, default_config->port.chars, config->port.size);
	}
	else
	{
		int_to_str(port, config->port.chars);
	};

	// max_http_routes

	config->max_http_routes = cjson_read_num(json, default_config->max_http_routes, "max_http_routes");

	// max_ws_routes

	config->max_ws_routes = cjson_read_num(json, default_config->max_ws_routes, "max_ws_routes");

	// allow_credentials

	config->allow_credentials = cjson_read_bool(json, default_config->allow_credentials, "allow_credentials");

	// allow_origins

	int ao_res = cjson_read_arr_string(json, "allow_origins", allow_origins_add, config->allow_origins);
	if (ao_res == -1 || array_is_empty(config->allow_origins))
	{
		config->allow_origins = default_config->allow_origins; // also hits here when its "*"
	}

	// allow_headers

	int ah_res = cjson_read_arr_string(json, "allow_headers", allow_headers_add, config->allow_headers);
	if (ah_res == -1 || array_is_empty(config->allow_headers))
	{
		config->allow_headers = default_config->allow_headers;
	}

	// allow_methods

	int am_res = cjson_read_arr_string(json, "allow_methods", allow_methods_add, config->allow_methods);
	if (am_res == -1 || array_is_empty(config->allow_methods))
	{
		config->allow_methods = default_config->allow_methods;
	}

	// secure

	config->secure = cjson_read_bool(json, default_config->secure, "secure");

	cJSON_Delete(json);
	return 0;
}

/**
 * @brief Loads config from .json file, else fills config with default values from default_config
 * @param path path of configuration file location
 * @return
 */
ServerConfig *config_init(const char *path)
{

	int res = config_init_inner(path);
	if (res == -1)
	{
		g_config = g_default_config;
		g_config.resource_path = path;
	}
	return &g_config;
}

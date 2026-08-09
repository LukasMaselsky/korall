#include "http_routes.h"

static Array g_http_routes = array_create_stack(NULL, sizeof(HTTPRoute), 0, 0);
static Array g_ws_routes = array_create_stack(NULL, sizeof(WebsocketRoute), 0, 0);

Array *http_routes_get()
{
	return &g_http_routes;
}

Array *ws_routes_get()
{
	return &g_ws_routes;
}

void http_routes_init(ServerConfig *config)
{
	size_t capacity = sizeof(HTTPRoute) * config->max_http_routes;
	Array *routes = &g_http_routes;
	routes->data = (uint8_t *)exit_calloc(capacity, sizeof(HTTPRoute));
	routes->capacity = capacity;
}

void ws_routes_init(ServerConfig *config)
{
	size_t capacity = sizeof(WebsocketRoute) * config->max_ws_routes;
	Array *routes = &g_ws_routes;
	routes->data = (uint8_t *)exit_calloc(capacity, sizeof(WebsocketRoute));
	routes->capacity = capacity;
}

void routes_init(ServerConfig *config)
{
	http_routes_init(config);
	ws_routes_init(config);
}

static void routes_free_one(Array *routes)
{
	if (routes == NULL)
		return;
	free(routes->data);
}

void routes_free(Array *http, Array *ws)
{
	routes_free_one(http);
	routes_free_one(ws);
}
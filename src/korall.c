#include "korall/korall.h"
#include "socket/socket.h"
#include "http/http.h"
#include "arena/arena.h"
#include "lookup/lookup_tables.h"
#include "cJSON/cJSON.h"
#include "config/config.h"
#include "gui/gui.h"
#include "server/http/server_http.h"
#include "http/tls/tls.h"
#include "http/routes/http_routes.h"

// https://stackoverflow.com/questions/58885831/what-does-reaping-children-imply
// https://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static SSL_CTX *ssl_ctx_init(ServerConfig *config)
{
	SSL_CTX *ssl_ctx = NULL;
	if (config->secure)
	{
		openssl_init();

		ssl_ctx = openssl_create_server_ctx(config->resource_path);
		if (ssl_ctx == NULL)
		{
			KORALL_LOG(LOG_ERR, "failed to create server SSL_CTX\n");
		}
		else
		{
			KORALL_LOG(LOG_INFO, "created server SSL_CTX\n");
		}
	}
	return ssl_ctx;
}

static SOCKET server_socket_init()
{
	int res = socket_init();
	if (res != 0)
	{
		KORALL_LOG(LOG_ERR, "socket initialisation failed, exiting\n");
		exit(EXIT_FAILURE);
	}

	SOCKET server_sock = init_listen_socket();
	if (server_sock == INVALID_SOCKET)
	{
		exit(EXIT_FAILURE);
	}
	return server_sock;
}

static void cleanup(SOCKET server_sock, SSL_CTX *ssl_ctx, ServerConfig *config)
{
	socket_close(server_sock);

	KORALL_LOG(LOG_INFO, "closed socket\n");

	socket_quit();

	// cleanup

	if (ssl_ctx != NULL)
	{
		SSL_CTX_free(ssl_ctx);
	}
	logging_cleanup();
	openssl_cleanup();
	Array *routes = http_routes_get();
	Array *ws_routes = ws_routes_get();
	routes_free(routes, ws_routes);
	config_free(config);
}

static void server_run(unsigned long gui_thread_id, SOCKET server_sock, SSL_CTX *ssl_ctx)
{
	ThreadState threads[MAX_THREADS] = {0};
	Array thread_arr = array_create_stack(threads, sizeof(ThreadState), 0, MAX_THREADS);

	Array *http_routes = http_routes_get();
	Array *ws_routes = ws_routes_get();

	ProcessDataArgs args = {
		.sock = 0,
		.http_routes = http_routes,
		.ws_routes = ws_routes,
		.thread_num = 0,
		.thread_arr = &thread_arr,
		.lock = &lock,
		.ssl = NULL,
		.gui_thread_id = gui_thread_id};

	SOCKET sock;

	while (true)
	{
		// todo: ensure 1 thread per connection (if doing http://localhost:3500 req then this happens when TLS fails)
		SSL *ssl = NULL;
		sock = process_incoming_connection(server_sock, ssl_ctx, &ssl);
		if (sock == INVALID_SOCKET)
			continue;

		sync_threads(&thread_arr, &lock);

		// spawn 1 thread for each connection

		// ! no mutex needed here since threads don't change array size

		if (array_full(&thread_arr))
		{
			KORALL_LOG(LOG_ERR, "not enough threads (max %d), could not process connection\n", MAX_THREADS);
			continue;
		}

		// make copy of args

		ProcessDataArgs *t_args = exit_calloc(1, sizeof(ProcessDataArgs));
		memcpy(t_args, &args, sizeof(*t_args));
		t_args->sock = sock;
		t_args->thread_num = thread_arr.size;
		t_args->ssl = ssl;

		create_data_process_thread(&thread_arr, t_args);
	}
}

//

void korall_http_routes_add(const char *path, const char *method, void (*const callback)(const HTTPRequest *, HTTPResponse *))
{
	Array *routes = http_routes_get();
	if (path == NULL)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, path cannot be NULL\n");
		return;
	}
	if (callback == NULL)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, callback cannot be NULL\n");
		return;
	}
	HTTPMethod meth_int = lookup_str_int(method, &http_method_lookup_table, true);
	if (meth_int == -1)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, method is not valid\n");
		return;
	}
	const HTTPRoute route = {.path = path, .method = meth_int, .callback = callback};
	if (array_push(routes, &route) == -1)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, maximum route count exceeded\n");
	}
}

void korall_ws_routes_add(const char *path, void (*const callback)(const WebsocketFrame *))
{
	Array *routes = ws_routes_get();
	if (path == NULL)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, path cannot be NULL\n");
		return;
	}
	if (callback == NULL)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, callback cannot be NULL\n");
		return;
	}
	const WebsocketRoute route = {.path = path, .callback = callback};
	if (array_push(routes, &route) == -1)
	{
		KORALL_LOG(LOG_ERR, "failed to add route, maximum route count exceeded\n");
	}
}

/**
 * @brief
 *
 * @param config_path
 * @param log_file can be stdout
 */
void korall_init(const char *config_path, const FILE *log_file)
{

	logging_init(log_file);
	ServerConfig *config = config_init(config_path);
	routes_init(config);
}

void korall_run()
{

	ServerConfig *g_config = config_get();

	SSL_CTX *ssl_ctx = ssl_ctx_init(g_config);

	SOCKET server_sock = server_socket_init();

	unsigned int gui_thread_id = 0;
#if 0
	THREAD_T gui_thread;
	gui_run(&gui_thread, &gui_thread_id);
#endif
	server_run(gui_thread_id, server_sock, ssl_ctx);

	cleanup(server_sock, ssl_ctx, g_config);

	return;
}

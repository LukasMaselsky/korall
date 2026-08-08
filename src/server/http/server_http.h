#ifndef KORALL_SERVER_HTTP_H
#define KORALL_SERVER_HTTP_H

#include <stdlib.h>
#include "http/http_internal.h"
#include "http/websocket/websocket_internal.h"
#include "array/array.h"
#include "korall/korall.h"
#include "thread/thread.h"
#include "http/routes/http_routes.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "msg_queue/msg_queue.h"

#define READ_BUFFER_LEN KILOBYTE
#define RESPONSE_BUFFER_LEN KILOBYTE

#define HTTP_ROUTES_CAPACITY 100
#define WS_ROUTES_CAPACITY 100

#define MAX_THREADS 32 // todo: get system ?

typedef struct
{
	size_t thread_num;
	pthread_mutex_t *lock;
	Array *thread_arr;
	SOCKET sock;
	Array *http_routes;
	Array *ws_routes;
	SSL *ssl;
	unsigned long gui_thread_id;
} ProcessDataArgs;

typedef struct {
	const SOCKET socket;
	const char* data;
	size_t data_byte_len;
	const SSL* ssl;
} IncomingRequestInfo;

//

HTTPRoute *http_route_select(HTTPRequest *req, const Array *routes);

WebsocketRoute *ws_route_select(HTTPRequest *req, const Array *routes);

bool http_domain_port_match_server(const HTTPRequest *req, const ServerConfig* config);

bool websocket_process_data(
	const IncomingRequestInfo* req_info,
	const WebsocketRoute *route,
	const MessageQueueWriteInfo *mqi
);

int route_not_found(HTTPResponse *res, const ServerConfig *config);

bool http_process_request(
	const IncomingRequestInfo* req_info,
	const Array *routes,
	const Array *ws_routes,
	bool *is_websocket,
	WebsocketRoute **websocket_route,
	const MessageQueueWriteInfo* mqi
);

SOCKET init_listen_socket();

SOCKET process_incoming_connection(SOCKET sock, SSL_CTX *ssl_ctx, SSL **ssl_p);

void process_incoming_data(void *arg);

void sync_threads(Array *thread_arr, pthread_mutex_t *lock);

void create_data_process_thread(Array *thread_arr, ProcessDataArgs *t_args);

#endif
#ifndef KORALL_MSG_QUEUE_H
#define KORALL_MSG_QUEUE_H

#include "utils/utils.h"
#include "http/http_internal.h"
#include "korall/websocket.h"
#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#else
typedef int mqd_t; // doesn't matter, not used
#endif

typedef enum {
	MSG_Q_READ,
	MSG_Q_WRITE,
} MessageQueueMode;

typedef enum {
	MSG_TYPE_REQ,
	MSG_TYPE_RES,
	MSG_TYPE_WS_FRAME,
} MessageType;

typedef struct {
	const HTTPRequest* req;
	const HTTPResponse* res;
	const WebsocketFrame* ws_frame;
	const MessageType type;
} Message;


int msg_queue_open(const char* queue_name, MessageQueueMode mode);

int msg_queue_close(const char* queue_name, mqd_t mq);

int msg_queue_post(mqd_t mqdes, unsigned long thread_id, Message* msg);

int msg_queue_read(mqd_t mqdes, Message* out);

#endif
#ifndef KORALL_MSG_QUEUE_H
#define KORALL_MSG_QUEUE_H

#include "utils/utils.h"
#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#else
typedef int mqd_t; // doesn't matter, not used
#endif

#define MAX_MSG_SIZE 10000 // todo: 

typedef enum {
	MSG_Q_READ,
	MSG_Q_WRITE,
} MessageQueueMode;

int msg_queue_post(mqd_t mqdes, unsigned long thread_id, void* msg, size_t msg_len);

void msg_queue_read(mqd_t mqdes, void (* const callback)(void*));


#endif
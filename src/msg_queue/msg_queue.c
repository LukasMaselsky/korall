#include "msg_queue.h"

int msg_queue_open(const char *queue_name, MessageQueueMode mode) {
#ifndef _WIN32
	int oflag = mode == MSG_Q_READ ? (O_CREAT | O_RDONLY) : (O_WRONLY);

	mqd_t mq = mq_open(queue_name, O_CREAT | oflag, 0666, NULL);
	if (mq == (mqd_t)-1) {
		log_msg(LOG_ERR, "mq_open failed\n");
		return -1;
	}
	return 0;
#endif
}

int msg_queue_close() {
}

int msg_queue_post(mqd_t mqdes, unsigned long thread_id, void *msg, size_t msg_len) {

#ifdef _WIN32
	void* msg_copy = safe_calloc(1, msg_len);
	memcpy(msg_copy, msg, msg_len); // todo: free

	bool ret = PostThreadMessage(thread_id, 0, 0, msg_copy);
	if (ret == 0) {
		DWORD err = GetLastError();
		
		switch (err) {
			case ERROR_INVALID_THREAD_ID:
				log_msg(LOG_ERR, "could not post message to queue, thread id invalid or thread does not have a message queue\n");
				break;
			case ERROR_NOT_ENOUGH_QUOTA:
				log_msg(LOG_ERR, "could not post message to queue, message limit hit\n");
				break;
			default:
				log_msg(LOG_ERR, "could not post message to queue\n");
				break;
		}
		return -1;
	}
#else
	if (mq_send(mqdes, msg, msg_len, 0) == -1) {
		log_msg(LOG_ERR, "could not post message to queue\n")
		return -1;
	}
#endif
	log_msg(LOG_INFO, "message posted to queue\n");
	return 0;
}

void msg_queue_read(mqd_t mqdes, void (* const callback)(void*)) {
#ifdef _WIN32
	MSG msg = { 0 };
	// If hWnd is -1, PeekMessage retrieves only messages on the current thread's message queue whose hwnd value is NULL, that is, thread messages as posted by PostMessage (when the hWnd parameter is NULL) or PostThreadMessage.
	while (PeekMessage(&msg, -1, 0, 0, PM_REMOVE)) {
		callback((void*)msg.lParam);
	}
	
#else
	if (mqdes == (mqd_t)-1) {
		log_msg(LOG_ERR, "mqdes cannot be invalid\n");
		return;
	};

	char msg[MAX_MSG_SIZE] = { 0 };

	ssize_t bytes_read = mq_receive(mqdes, msg, MAX_MSG_SIZE, NULL);
	if (bytes_read == -1) {
		log_msg(LOG_ERR, "mq_receive failed\n");
		return;		
	}

	callback((void*)msg);

#endif
	return;
}
#include "thread.h"
#include "utils/utils.h"

int thread_create(THREAD_T *thread, void (* const func)(void *), void* arg) {
#ifdef _WIN32
	uintptr_t btx = _beginthreadex(NULL, 0, func, arg, 0, NULL);
	if (btx == 0) {
		log_msg(LOG_ERR, "failed to create thread, could not process connection\n");
		return -1;
	}
	*thread = (THREAD_T)btx;
#else
	int res = pthread_create(thread, NULL, func, arg);
	if (res != 0) {
		log_msg(LOG_ERR, "failed to create thread, could not process connection\n");
		return -1;
	}
#endif
	return 0;
}
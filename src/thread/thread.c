#include "thread.h"
#include "utils/utils.h"

int thread_create(THREAD_T *thread, void (*const func)(void *), void *arg, unsigned int *thread_id)
{
#ifdef _WIN32
	uintptr_t btx = _beginthreadex(NULL, 0, (_beginthreadex_proc_type)func, arg, 0, thread_id);
	if (btx == 0)
	{
		KORALL_LOG(LOG_ERR, "failed to create thread, could not process connection\n");
		return -1;
	}
	*thread = (THREAD_T)btx;
#else
	int res = pthread_create(thread, NULL, (void *(*)(void *))func, arg);
	if (res != 0)
	{
		KORALL_LOG(LOG_ERR, "failed to create thread, could not process connection\n");
		return -1;
	}
#endif
	return 0;
}

// PostThreadMessage

// PeekMessage
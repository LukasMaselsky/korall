#ifndef KORALL_THREAD_H
#define KORALL_THREAD_H

#include <stdbool.h>

#ifdef _WIN32
	#include <WinSock2.h>
	typedef HANDLE THREAD_T;
	typedef int pthread_mutex_t; // type not important
	#define PTHREAD_MUTEX_INITIALIZER 0 // not important
#else
	#include <sys/types.h>
	#include <pthread.h>
	typedef pthread_t THREAD_T;

#endif

typedef struct {
	THREAD_T thread;
	bool running;
} ThreadState;

#endif
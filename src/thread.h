#ifndef KORALL_THREAD_H
#define KORALL_THREAD_H


#ifdef _WIN32
	#include <WinSock2.h>
	#define THREAD_T HANDLE	

#else
	#include <sys/types.h>
	#include <pthread.h>
	#define THREAD_T pthread_t

#endif

#endif
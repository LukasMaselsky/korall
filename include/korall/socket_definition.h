#ifndef SOCKET_DEFINITION_H
#define SOCKET_DEFINITION_H

#ifdef _WIN32
	#include <winsock2.h>
#else
	typedef int SOCKET
#endif

#endif
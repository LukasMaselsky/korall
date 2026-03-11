#if defined HEADERS
#include "sockets.h"
#elif defined TESTS

TEST("is_valid_ipv4") {
	ASSERT(is_valid_ipv4("0.0.0.0"));
	ASSERT(is_valid_ipv4("192.3.45.1"));
	ASSERT(!is_valid_ipv4("256.3.45.1"));
	ASSERT(!is_valid_ipv4("-10.3.45.1"));
	ASSERT(!is_valid_ipv4("000.0.0.0"));
	ASSERT(!is_valid_ipv4(".0.0.0"));
	ASSERT(!is_valid_ipv4("0..0.0"));
}
#endif
#if defined HEADERS
#include <string.h>
#include "utils.h"
#elif defined TESTS
TEST("strlen returns length of string") {
	ASSERT(strlen("") == 0);
	ASSERT(strlen("foo") == 3);
}

TEST("demonstrate a failing test") {
	bool a = is_digits_only("123");
	ASSERT(a == true);
}
#endif
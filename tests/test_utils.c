#if defined HEADERS
#include <string.h>
#include "utils.h"
#elif defined TESTS

TEST("strlen returns length of string") {
	ASSERT(strlen("") == 0);
	ASSERT(strlen("foo") == 3);
}

TEST("is_digits_only") {
	bool res;
	res = is_digits_only("a");
	ASSERT(!res);

	res = is_digits_only("1");
	ASSERT(res);
	
	res = is_digits_only("1a");
	ASSERT(!res);

	res = is_digits_only("1736291927437");
	ASSERT(res);

	res = is_digits_only("17362X1927437");
	ASSERT(!res);

	res = is_digits_only("");
	ASSERT(!res);

}

#endif
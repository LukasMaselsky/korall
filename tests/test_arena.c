#if defined HEADERS
#include "http/http.h"
#include "lookup/lookup_tables.h"
#include "arena/arena.h"

#elif defined TESTS

TEST("arena")
{
	Arena arena = arena_init(1000);
	char *str1 = (char *)arena_alloc(&arena, 5);
	strcpy(str1, "abcd");
	char *str2 = (char *)arena_alloc(&arena, 5);
	strcpy(str2, "efgh");

	// printf("str1: %s, str2: %s\n", str1, str2);

	arena_free(&arena);
}
#endif
#include <stdio.h>
#define HEADERS
#include "test_all.c"
#undef HEADERS

#define TEST(name) test = name; test_count++; printf("Running test: %s\n", name);
#define ASSERT(ast)\
  do {\
    assertion = #ast;\
    file = __FILE__;\
    line = __LINE__;\
    if(!(ast)) goto fail;\
  } while(0)

int main() {
    const char* test = "";
    const char* assertion = "";
    const char* file = "";
    int line = 0;
    int test_count = 0;

# define TESTS
# include "test_all.c"
# undef TESTS

    printf("\n");
    printf(ANSI_COLOR_GREEN "All tests passed (%d/%d)\n" ANSI_COLOR_RESET, test_count, test_count);
    return 0;

fail:
    printf("\n");
    printf(ANSI_COLOR_RED "Test failed at %s:%d\n    %s: %s" ANSI_COLOR_RESET "\n",
        file, line,
        test, assertion);
    return -1;
}
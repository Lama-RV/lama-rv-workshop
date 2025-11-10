#include <cstdio>
#include <cstdlib>

#ifdef DEBUG
#define ASSERT(condition, code, ...)   \
    do {                               \
        if (!(condition))              \
            FAIL((code), __VA_ARGS__); \
    } while (0)
#else
#define ASSERT(condition, code, ...)
#endif

#define FAIL(code, ...)                                                          \
    do {                                                                         \
        static_assert((code), "Expected non-zero integer literal");              \
        fprintf(stderr, "Failed at line %d with code %d\n\t", __LINE__, (code)); \
        fprintf(stderr, __VA_ARGS__);                                            \
        fprintf(stderr, "\n");                                                   \
        exit(code);                                                              \
    } while (0)

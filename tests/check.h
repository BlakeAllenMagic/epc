#ifndef CHECK_H
#define CHECK_H

//need stdio for printf, stdint for fixed width types
#include <stdio.h>
#include <stdint.h>

static int check_failures = 0;

// check argument is true
#define CHECK(arg) \
    do { \
        if (!(arg)) { \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #arg); \
            check_failures++; \
        } \
    } while (0)

// check if value equals another
#define CHECK_EQ(actual, expected) \
    do { \
        unsigned long _a = (unsigned long)(actual); \
        unsigned long _e = (unsigned long)(expected); \
        if (_a != _e) { \
            printf("FAIL %s:%d: %s: expected 0x%lX, got 0x%lX\n", \
                __FILE__, __LINE__, #actual, _e, _a); \
            check_failures++; \
        } \
    } while (0)

// report checks
static int check_report(void)
{
    if (check_failures > 0) {
        printf("%d check(s) failed\n", check_failures);
        return 1;
    }
    printf("all checks passed\n");
    return 0;
}

#endif /* CHECK_H */
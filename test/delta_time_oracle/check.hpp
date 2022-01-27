#pragma once

#include <stdio.h>
#include <stdlib.h>

#define check(expr)                                                           \
    do {                                                                      \
        if (!(expr)) {                                                        \
            fprintf(stderr, "Error line %d: %s is false\n", __LINE__, #expr); \
            abort();                                                          \
        }                                                                     \
    } while (false)


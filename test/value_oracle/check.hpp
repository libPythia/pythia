#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define check(v, expr)                                               \
    do {                                                             \
        if ((expr) != (v)) {                                         \
            fprintf(stderr,                                          \
                    "Error line %d: %s is %d but %d was expected\n", \
                    __LINE__,                                        \
                    #expr,                                           \
                    expr,                                            \
                    v);                                              \
            assert(false);                                           \
        }                                                            \
    } while (false)


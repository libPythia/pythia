#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "thread_id.hpp"

#if 0
#    define check(x)                                                                     \
        do {                                                                             \
            if (!(x)) {                                                                  \
                fprintf(stderr, "'%s' check failed at %s:%d\n", #x, __FILE__, __LINE__); \
                abort();                                                                 \
            }                                                                            \
        } while (false);
#else
#    define check(x) \
        do {         \
        } while (false)
#endif

#if 0
#    define log(x)                                                    \
        do {                                                          \
            fprintf(stderr, "#%d %s\n", pythia_get_thread_num(), #x); \
            fflush(stderr);                                           \
        } while (false)

struct LogFn {
    LogFn(char const * name) : _name(name) {
        fprintf(stderr, "#%d enter %s\n", pythia_get_thread_num(), name);
        fflush(stderr);
    }
    ~LogFn() {
        fprintf(stderr, "#%d exit %s\n", pythia_get_thread_num(), _name);
        fflush(stderr);
    }
    char const * _name;
};

#    define log_fn \
        auto const _log_fn_ = LogFn { __PRETTY_FUNCTION__ }
//__FUNCTION__, "/ " __FUNCTION__)
#else
#    define log(x) \
        do {       \
        } while (false)
#    define log_fn
#endif

#pragma once

#include <pthread.h>

#include "settings.hpp"

// Dont default initialize the pointer or they will be reseted to default value AFTER init
struct FunctionPointers {
    void * (*calloc)(size_t, size_t);
    void * (*malloc)(size_t);
    void (*free)(void *);
    void * (*realloc)(void *, size_t);
    int (*pthread_create)(pthread_t *,
                          pthread_attr_t const *,
                          void * (*start_routine)(void *),
                          void *);

    void * (*_calloc)(size_t, size_t);
    void * (*_malloc)(size_t);
    void (*_free)(void *);
    void * (*_realloc)(void *, size_t);

    static void * calloc_rec_mem(size_t nmemb, size_t size);
    static void * malloc_rec_mem(size_t size);
    static void free_rec_mem(void * ptr);
    static void * realloc_rec_mem(void * ptr, size_t size);

    int allocation_record_output;

    auto init(Settings const *) -> void;
    auto deinit(Settings const *) -> void;
};

inline FunctionPointers fn_ptr;

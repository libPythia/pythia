#include "interceptor.hpp"

#include <dlfcn.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "allocator.hpp"
#include "debug.hpp"
#include "settings.hpp"
#include "thread_id.hpp"
#include "tmp_allocator.hpp"

// ---------------------------------------------------

enum class LibStatus {
    Disabled,
    Uninitialized,
    Initializing,
    Initialized,
    Terminated,
};

static auto lib_status = LibStatus::Uninitialized;

// ---------------------------------------------------

static thread_local auto recursion_count = 0;

// ---------------------------------------------------

auto FunctionPointers::init(Settings const & settings) -> void {
    log_fn;

    fn_ptr._calloc = (void * (*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    fn_ptr._malloc = (void * (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    fn_ptr._realloc = (void * (*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    fn_ptr._free = (void (*)(void *))dlsym(RTLD_NEXT, "free");

    if (settings.record_allocations) {
        log(fn_ptr::init record);
        calloc = &calloc_rec_mem;
        malloc = &malloc_rec_mem;
        free = &free_rec_mem;
        realloc = &realloc_rec_mem;

        allocation_record_output = open(settings.allocations_file, O_CREAT | O_TRUNC | O_WRONLY);
        check(allocation_record_output >= 0);
    } else {
        log(fn_ptr::init dont record);
        calloc = _calloc;
        malloc = _malloc;
        free = _free;
        realloc = _realloc;

        allocation_record_output = -1;
    }
}

auto FunctionPointers::deinit(Settings const & settings) -> void {
    log_fn;
    if (settings.record_allocations)
        close(allocation_record_output);
}

static auto print_mem(char sign, void * ptr) {
    char buf[256];
    auto const size = sprintf(buf, "## %c%lu\n", sign, malloc_usable_size(ptr));
    write(fn_ptr.allocation_record_output, buf, size);
}

void * FunctionPointers::calloc_rec_mem(size_t nmemb, size_t size) {
    log_fn;
    auto const ptr = fn_ptr._calloc(nmemb, size);
    print_mem('+', ptr);
    return ptr;
}

void * FunctionPointers::malloc_rec_mem(size_t size) {
    log_fn;
    auto const ptr = fn_ptr._malloc(size);
    print_mem('+', ptr);
    return ptr;
}

void FunctionPointers::free_rec_mem(void * ptr) {
    log_fn;
    fprintf(stderr, "%p\n", ptr);
    print_mem('-', ptr);
    fn_ptr._free(ptr);
}

void * FunctionPointers::realloc_rec_mem(void * ptr, size_t size) {
    log_fn;
    print_mem('-', ptr);
    ptr = fn_ptr._realloc(ptr, size);
    print_mem('+', ptr);
    return ptr;
}

// ---------------------------------------------------

static void init() {
    log_fn;
    auto const settings = get_settings();

    check(lib_status == LibStatus::Uninitialized);
    check(recursion_count == 0);
    lib_status = LibStatus::Initializing;

    fn_ptr.init(settings);

    // From here, we want to perform real allocation, but dont want to start tracing
    recursion_count = 1;

    fn_ptr.pthread_create =
            (int (*)(pthread_t *, pthread_attr_t const *, void * (*)(void *), void *))dlsym(
                    RTLD_NEXT,
                    "pthread_create");

    Allocator::init(settings);

    if (settings.mode == Mode::Disabled)
        lib_status = LibStatus::Disabled;
    else
        lib_status = LibStatus::Initialized;

    recursion_count = 0;
}

static void __attribute__((constructor)) on_init(void) {
    log_fn;
    if (lib_status == LibStatus::Uninitialized)
        init();
}

static void __attribute__((destructor)) on_deinit(void) {
    log_fn;
    lib_status = LibStatus::Disabled;
    auto const settings = get_settings();
    Allocator::deinit(settings);
    fn_ptr.deinit(settings);
}

// ---------------------------------------------------

struct RecursionShield {
    RecursionShield() { ++recursion_count; }
    ~RecursionShield() { --recursion_count; }
    auto is_in_recursion() const -> bool { return recursion_count > 1; }
};

extern "C" {
void * calloc(size_t nmemb, size_t size) {
    switch (lib_status) {
        case LibStatus::Initializing: {
            auto const total_size = nmemb * size;
            if (total_size == 0)
                return nullptr;
            auto const ptr = (char *)eta_tmp_malloc(total_size);
            for (auto i = 0u; i < total_size; ++i)
                ptr[i] = 0;
            return ptr;
        }
        case LibStatus::Uninitialized: {
            init();
            return calloc(nmemb, size);
        }
        case LibStatus::Initialized: {
            auto shield = RecursionShield {};
            if (shield.is_in_recursion())
                return fn_ptr._calloc(nmemb, size);
            return Allocator::calloc(nmemb, size);
        }
        case LibStatus::Disabled: {
            auto shield = RecursionShield {};
            if (shield.is_in_recursion())
                return fn_ptr._calloc(nmemb, size);
            return fn_ptr.calloc(nmemb, size);
        }
        case LibStatus::Terminated: [[fallthrough]];
        default: check(false);
    }
    return nullptr;
}

void * malloc(size_t size) {
    switch (lib_status) {
        case LibStatus::Initializing:  // during init()
            return eta_tmp_malloc(size);
        case LibStatus::Uninitialized:  // First call of malloc, before on_init() !
            init();
            return malloc(size);
        case LibStatus::Initialized: {
            auto const shield = RecursionShield {};
            if (shield.is_in_recursion())
                return fn_ptr._malloc(size);
            return Allocator::malloc(size);
        }
        case LibStatus::Disabled: {
            auto const shield = RecursionShield {};
            if (shield.is_in_recursion())
                return fn_ptr._malloc(size);
            return fn_ptr.malloc(size);
        }
        case LibStatus::Terminated: [[fallthrough]];  // should not be here
        default: check(false);
    }
    return nullptr;
}

void free(void * ptr) {
    switch (lib_status) {
        case LibStatus::Initializing:  // during init()
            return eta_tmp_free(ptr);
        case LibStatus::Initialized: {
            auto const shield = RecursionShield {};
            if (shield.is_in_recursion())
                fn_ptr._free(ptr);
            else
                Allocator::free(ptr);
        } break;
        case LibStatus::Uninitialized:  // First call of free before malloc or realloc ?
            check(ptr == nullptr);
            break;
        case LibStatus::Disabled: {
            auto const shield = RecursionShield {};
            if (shield.is_in_recursion())
                fn_ptr._free(ptr);
            fn_ptr.free(ptr);
        } break;
        case LibStatus::Terminated: [[fallthrough]];  // should not be here
        default: check(false);
    }
}

void * realloc(void * ptr, size_t size) {
    switch (lib_status) {
        case LibStatus::Initializing:  // during init()
            return eta_tmp_malloc(size);
        case LibStatus::Uninitialized:  // First call of malloc, before on_init() !
            init();
            return realloc(ptr, size);
        case LibStatus::Initialized: {
            auto const shield = RecursionShield {};
            if (shield.is_in_recursion())
                return fn_ptr._realloc(ptr, size);
            return Allocator::realloc(ptr, size);
        }
        case LibStatus::Disabled: {
            auto const shield = RecursionShield {};
            if (shield.is_in_recursion())
                return fn_ptr._realloc(ptr, size);
            return fn_ptr.realloc(ptr, size);

        } break;
        case LibStatus::Terminated: [[fallthrough]];  // should not be here
        default: check(false);
    }
    return nullptr;
}

// ---------------------------------------------------

struct NewThreadInfos {
    int num;
    void * (*start_routine)(void *);
    void * arg;
};

static NewThreadInfos new_threads_infos[pythia_max_num_threads];

static auto new_start_routine(void * arg) -> void * {
    auto const thread_infos = static_cast<NewThreadInfos *>(arg);
    pythia_set_thread_num(thread_infos->num);
    return thread_infos->start_routine(thread_infos->arg);
}

int pthread_create(pthread_t * thread,
                   const pthread_attr_t * attr,
                   void * (*start_routine)(void *),
                   void * arg) {
    switch (lib_status) {
        case LibStatus::Uninitialized: init(); [[fallthrough]];
        case LibStatus::Initialized: [[fallthrough]];
        case LibStatus::Terminated: {
            check(fn_ptr.pthread_create != nullptr);
            fprintf(stderr, "#%d, pthread_create(...);\n", pythia_get_thread_num());

            auto const new_thread_num = pythia_new_thread_num();
            auto & thread_infos = new_threads_infos[new_thread_num];
            thread_infos.num = new_thread_num;
            thread_infos.start_routine = start_routine;
            thread_infos.arg = arg;
            return fn_ptr.pthread_create(thread, attr, new_start_routine, &thread_infos);
        }
        case LibStatus::Disabled: return fn_ptr.pthread_create(thread, attr, start_routine, arg);
        case LibStatus::Initializing: [[fallthrough]];
        default: check(false);
    }
}

}  // extern "C"

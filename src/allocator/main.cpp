#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>

#include <iostream>

// ------------------------------------------------------

// static bool is_in_recursion = false;
// struct recursive_shield {
//     recursive_shield() { is_in_recursion = true; }
//     ~recursive_shield() { is_in_recursion = false; }
// };

// ------------------------------------------------------

static void * (*real_malloc)(size_t) = nullptr;
static void (*real_free)(void *) = nullptr;
static void * (*real_realloc)(void *, size_t) = nullptr;

// ------------------------------------------------------

static void __attribute__((constructor)) init(void) {
    // real_malloc = (void * (*)(size_t))dlsym(RTLD_NEXT, "malloc");
    // real_realloc = (void * (*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    // must be done after malloc because
    // malloc wait real_free to be set to really allocate memory
    // real_free = (void (*)(void *))dlsym(RTLD_NEXT, "free");

    // assert(real_malloc != nullptr);
    // assert(real_realloc != nullptr);
    // assert(real_free != nullptr);

    // TODO prepare oracle
}

static void __attribute__((destructor)) deinit(void) {
    // TODO compute and save
}

// ------------------------------------------------------

static auto constexpr tmp_allocation_buffer_size = 128;
static char tmp_allocation_buffer[tmp_allocation_buffer_size];
static auto is_tmp_allocation_buffer_used = false;

void * malloc(size_t size) {
    // if (real_free == nullptr) {  // dont alloc if we cannot free !
    // assert(size <= tmp_allocation_buffer_size);
    // assert(is_tmp_allocation_buffer_used == false);
    // is_tmp_allocation_buffer_used = true;
    return tmp_allocation_buffer;
    // } else {
    //     auto ptr = real_malloc(size);
    //     // if (is_in_recursion) {
    //     //     auto shield = recursive_shield {};
    //     //     std::cout << "malloc(" << size << ") -> " << ptr << ";" << std::endl;
    //     // }
    //     return ptr;
    // }
}

void free(void * ptr) {
    if (real_free == nullptr) {
        // assert(ptr == tmp_allocation_buffer);
        // assert(is_tmp_allocation_buffer_used == true);
        // is_tmp_allocation_buffer_used = false;
    } else {
        // if (is_in_recursion) {
        //     auto shield = recursive_shield {};
        //     std::cout << "free(" << ptr << ");" << std::endl;
        // }
        // real_free(ptr);
    }
}

void * realloc(void * ptr, size_t size) {
    // assert(real_free != nullptr);
    // auto res = real_realloc(ptr, size);
    // if (is_in_recursion) {
    //     auto shield = recursive_shield {};
    //     std::cout << "realloc(" << ptr << ", " << size << ") -> " << res << ";" << std::endl;
    // }
    // return res;
    return ptr;
}

// ------------------------------------------------------


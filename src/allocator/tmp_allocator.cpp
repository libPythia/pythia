#include "tmp_allocator.hpp"

#include "debug.hpp"

static auto constexpr max_size_allocation = 64 * 256 * 256;
static auto constexpr max_allocation_count = 16;

static char buffers[max_size_allocation][max_allocation_count];
static auto allocation_count = 0;

auto eta_tmp_malloc(size_t size) -> void * {
    log_fn;
    check(size <= max_size_allocation);
    check(allocation_count <= max_allocation_count);
    return buffers[allocation_count++];
}

auto eta_tmp_free(void *) -> void {
    log_fn;
    // Do nothing
}


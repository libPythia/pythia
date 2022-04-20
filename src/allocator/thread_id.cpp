#include "thread_id.hpp"

#include <unistd.h>

#include <atomic>

#include "debug.hpp"

// ----------------------------------------------------------------

static auto num_threads = std::atomic<int> { 1 };
static auto constexpr invalid_thread_num = -1;
static thread_local auto thread_num = invalid_thread_num;

// ----------------------------------------------------------------

auto pythia_new_thread_num() -> int { return num_threads++; }

auto pythia_set_thread_num(int num) -> void {
    check(thread_num == invalid_thread_num);
    thread_num = num;
}

auto pythia_get_thread_num() -> int {
    if (getpid() == gettid())
        return 0;
    check(thread_num != invalid_thread_num);
    return thread_num;
}

auto pythia_get_num_threads() -> int { return num_threads; }


#include "allocator.hpp"

#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/prediction.hpp>
#include <eta/factorization/reduction.hpp>
#include <fstream>
#include <new>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "debug.hpp"
#include "interceptor.hpp"
#include "prediction.hpp"
#include "settings.hpp"
#include "thread_id.hpp"

// ---------------------------------------------------

static auto constexpr minimum_allocation_size_to_take_care = 1024;
static auto constexpr number_of_event_to_look_ahead = 10;
static Mode mode;

// ---------------------------------------------------

struct MemoryChunkSize {  // TODO rename
    std::vector<void *> reserve;
    Terminal * allocate_terminal;
    Terminal * deallocate_terminal;
};

struct alignas(64) ThreadAllocator {
    std::unordered_map<size_t, MemoryChunkSize> infos;
    std::unordered_map<void *, size_t> ptr_size;

    Grammar grammar;
    NonTerminal * root;
    Estimation estimation;

    Terminal * get_terminal(size_t size, Operation op) {
        auto const [it, inserted] =
                infos.try_emplace(size, MemoryChunkSize { {}, nullptr, nullptr });
        auto & infos = it->second;
        if (inserted) {
            infos.allocate_terminal =
                    new_terminal(grammar, new Event { Operation::Allocation, 0, size });
            infos.deallocate_terminal =
                    new_terminal(grammar, new Event { Operation::Deallocation, size, 0 });
        }

        switch (op) {
            case Operation::Allocation: return infos.allocate_terminal;
            case Operation::Deallocation: return infos.deallocate_terminal;
            case Operation::Reallocation: check(false);
        }
        return nullptr;
    }
};

// Need to use pointers here because it could be reseted after first use by
// static C++ object initializer

static auto get_thread_allocators() -> ThreadAllocator ** {
    static auto res = static_cast<ThreadAllocator **>(
            malloc(pythia_max_num_threads * sizeof(ThreadAllocator *)));
    return res;
}

static auto get_max_advance_for_prevision() -> size_t {
    static auto res = [] {
        auto i = static_cast<size_t>(10u);
        auto const env = getenv("PYTHIA_ADVANCE");
        if (env != nullptr)
            sscanf(env, "%lu", &i);
        return i;
    }();
    return res;
}

// ---------------------------------------------------

static auto get_thread_allocator() -> ThreadAllocator * {
    static thread_local auto const thread_allocator = []() -> ThreadAllocator * {
        auto & allocator = get_thread_allocators()[pythia_get_thread_num()];
        if (allocator == nullptr)
            allocator = new ThreadAllocator {};
        return allocator;
    }();
    return thread_allocator;
}

// ---------------------------------------------------

static void (*notify_allocation)(size_t);
static bool (*notify_deallocation)(size_t);

static auto record_allocation(size_t size) {
    auto const alloc = get_thread_allocator();
    auto const terminal = alloc->get_terminal(size, Operation::Allocation);
    alloc->root = insertSymbol(alloc->grammar, alloc->root, terminal);
}

static auto record_deallocation(size_t size) -> bool {
    auto const alloc = get_thread_allocator();
    auto const terminal = alloc->get_terminal(size, Operation::Deallocation);
    alloc->root = insertSymbol(alloc->grammar, alloc->root, terminal);
    return false;
}

static auto record_for_prediction_allocation(size_t size) -> void {
    auto const alloc = get_thread_allocator();
    auto const terminal = alloc->get_terminal(size, Operation::Allocation);
    fprintf(stderr, "Found no terminal for allocation of %lu bytes\n", size);
    alloc->estimation = next_estimation(alloc->estimation, terminal);
}

static auto is_an_allocation_about_to_occure(Prediction prediction,
                                             size_t size,
                                             int number_of_event_ahead) -> bool {
    // fprintf(stderr, "%d steps remaining\n", number_of_event_ahead);
    // fprintf(stderr, "estimation size : %lu\n", prediction.estimation.size());
    if (number_of_event_ahead > 0 && prediction.estimation.size() > 0) {
        do {
            auto const terminal = get_terminal(prediction);
            auto const event = static_cast<Event const *>(terminal->payload);
            switch (event->operation) {
                case Operation::Allocation: fprintf(stderr, "Observe allocation\n"); break;
                case Operation::Deallocation: fprintf(stderr, "Observe deallocation\n"); break;
            }
            if (event->operation == Operation::Allocation) {  // TODO condition on size
                fprintf(stderr, "Found allocation\n");
                return true;
            }
            auto next_prediction = prediction;
            get_first_next(&prediction);
            if (is_an_allocation_about_to_occure(std::move(next_prediction),
                                                 size,
                                                 number_of_event_ahead - 1)) {
                return true;
            }
        } while (get_alternative(&prediction));
    }
    fprintf(stderr, "Found no allocation\n");
    return false;
}

static auto predict_on_deallocation(size_t size) -> bool {
    auto const alloc = get_thread_allocator();
    auto const terminal = alloc->get_terminal(size, Operation::Deallocation);
    if (terminal == nullptr)
        fprintf(stderr, "Found no terminal for deallocation of %lu bytes\n", size);
    alloc->estimation = next_estimation(alloc->estimation, terminal);
    // fprintf(stderr, "Correspondance estimation size = %lu\n", alloc->estimation.size());
    auto const & prediction = get_prediction_from_estimation(alloc->estimation);
    // fprintf(stderr, "Start Predicting :\n");
    return is_an_allocation_about_to_occure(std::move(prediction),
                                            size,
                                            get_max_advance_for_prevision());
}

// ---------------------------------------------------

static auto get_file_path(Settings const * settings) -> std::string {
    return std::string(settings->trace_file) + std::string(settings->extension);
}

void Allocator::init(Settings const * settings) {
    log_fn;

    mode = settings->mode;

    switch (settings->mode) {
        case Mode::Predicting: {
            auto const alloc = get_thread_allocator();
            auto & grammar = alloc->grammar;
            auto const file_path = "pythia_allocator.btr";
            auto file = std::ifstream {};
            file.open(file_path, std::ifstream::binary);
            check(file.is_open());
            load_bin_file(grammar, file, deserialize);
            alloc->estimation = init_estimation_from_start(grammar);
            notify_allocation = &record_for_prediction_allocation;
            notify_deallocation = &predict_on_deallocation;

            for (auto const & terminal : alloc->grammar.terminals) {
                auto const event = static_cast<Event const *>(terminal->payload);
                switch (event->operation) {
                    case Operation::Allocation: {
                        auto const size = event->dest;
                        check(event->orig == 0);
                        auto & infos = alloc->infos[size];
                        infos.allocate_terminal = terminal.get();
                    } break;

                    case Operation::Deallocation: {
                        auto const size = event->orig;
                        check(event->dest == 0);
                        auto & infos = alloc->infos[size];
                        infos.deallocate_terminal = terminal.get();
                    } break;
                    case Operation::Reallocation: check(false); break;
                }
            }
        } break;
        case Mode::Recording: {
            auto const alloc = get_thread_allocator();
            notify_allocation = &record_allocation;
            notify_deallocation = &record_deallocation;
            alloc->root = nullptr;
        } break;
        default: break;
    }
}

auto Allocator::deinit(Settings const * settings) -> void {
    log_fn;

    if (settings->mode == Mode::Recording) {
        auto const num_threads = pythia_get_num_threads();
        // TODO export thread_num and save multiple traces
        for (auto i = 0; i < num_threads; ++i) {
            if (get_thread_allocators()[i] == nullptr) {
                check(i != 0);
                fprintf(stderr, "Thread %d was not used to allocate memory\n", i);
            } else {
                check(i == 0);
                auto const path = get_file_path(settings);
                fprintf(stderr, "Dump data for thread %d in file %s\n", i, path.c_str());

                auto & grammar = get_thread_allocator()->grammar;
                auto file = std::ofstream(path);
                print_bin_file(grammar, file, serialize);
            }
        }
    }
}

// ---------------------------------------------------

static auto internal_malloc(size_t size) -> void * {
    log_fn;
    notify_allocation(size);
    auto & data = get_thread_allocator()->infos[size];
    auto const ptr = [&]() {
        if (data.reserve.size() == 0)
            return fn_ptr.malloc(size);
        auto const p = data.reserve.back();
        data.reserve.pop_back();
        return p;
    }();
    // fprintf(stderr, "#%d malloc(%ld);\n", pythia_get_thread_num(), size);
    return ptr;
}

static auto internal_free(void * ptr, size_t size) -> void {
    log_fn;
    if (notify_deallocation(size)) {
        fprintf(stderr, "Keep for later\n");
        get_thread_allocator()->infos[size].reserve.push_back(ptr);

    } else {
        fprintf(stderr, "Free\n");
        fn_ptr._free(ptr);
    }
}

static auto internal_realloc(void * ptr, size_t prev_size, size_t size) -> void * {
    log_fn;
    notify_allocation(size);
    notify_deallocation(prev_size);
    return fn_ptr.realloc(ptr, size);  // TODO smart thing ?
}

static auto internal_calloc(size_t nmemb, size_t size) -> void * {
    log_fn;
    return internal_malloc(nmemb * size);
}

// ---------------------------------------------------

static auto set_size(void * ptr, size_t size) -> void {
    auto const [it, inserted] = get_thread_allocator()->ptr_size.try_emplace(ptr, size);
    check(inserted);
    it->second = size;
}

static auto get_and_remove_size(void * ptr) -> size_t {
    if (ptr == nullptr)
        return 0;
    auto & ptr_size = get_thread_allocator()->ptr_size;
    auto const it = ptr_size.find(ptr);
    // check(it != ptr_size.end());  // TODO
    if (it == ptr_size.end())
        return 0;
    auto const size = it->second;
    ptr_size.erase(it);
    return size;
}

// ---------------------------------------------------

auto Allocator::malloc(size_t size) -> void * {
    log_fn;
    if (mode != Mode::Disabled) {
        auto const ptr = [&]() {
            if (size < minimum_allocation_size_to_take_care)
                return fn_ptr.malloc(size);
            return internal_malloc(size);
        }();
        set_size(ptr, size);
        return ptr;
    }
    return fn_ptr.malloc(size);
}

auto Allocator::realloc(void * ptr, size_t size) -> void * {
    log_fn;
    if (mode != Mode::Disabled) {
        auto const prev_size = get_and_remove_size(ptr);
        auto const res = [&]() {
            if (std::max(size, prev_size) < minimum_allocation_size_to_take_care)
                return fn_ptr.realloc(ptr, size);
            return internal_realloc(ptr, prev_size, size);
        }();
        set_size(res, size);
        return res;
    }
    return fn_ptr.realloc(ptr, size);
}

auto Allocator::calloc(size_t nmemb, size_t size) -> void * {
    log_fn;
    if (mode != Mode::Disabled) {
        auto const ptr = [&]() {
            if (nmemb * size >= minimum_allocation_size_to_take_care)
                return internal_calloc(nmemb, size);
            return fn_ptr.calloc(nmemb, size);
        }();
        set_size(ptr, nmemb * size);
        return ptr;
    }
    return fn_ptr.calloc(nmemb, size);
}

auto Allocator::free(void * ptr) -> void {
    log_fn;
    if (mode != Mode::Disabled) {
        auto const size = get_and_remove_size(ptr);
        if (size >= minimum_allocation_size_to_take_care) {
            internal_free(ptr, size);
            return;
        }
    }
    fn_ptr.free(ptr);
}

// ---------------------------------------------------

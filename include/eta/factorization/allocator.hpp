#pragma once

#include <cassert>
#include <cstdlib>

namespace eta {

struct allocator_t {
    virtual ~allocator_t() = default;
    virtual auto alloc(std::size_t s) -> void * = 0;
    virtual auto dealloc(void * p) -> void = 0;
    virtual auto realloc(void * p, std::size_t s) -> void * = 0;
};

struct std_allocator_t : public allocator_t {
    auto alloc(std::size_t s) -> void * override { return std::malloc(s); }
    auto dealloc(void * p) -> void override { std::free(p); }
    auto realloc(void * p, std::size_t s) -> void * override { return std::realloc(p, s); }
};

inline std_allocator_t std_allocator;

struct fake_allocator_t : public allocator_t {
    auto alloc(std::size_t) -> void * override {
        assert(false);
        return nullptr;
    }
    auto dealloc(void *) -> void override { assert(false); }
    auto realloc(void *, std::size_t) -> void * override {
        assert(false);
        return nullptr;
    }
};

inline fake_allocator_t fake_allocator;

}  // namespace eta

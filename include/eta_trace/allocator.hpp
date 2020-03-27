#pragma once

#include <cstdlib>

struct GenericAllocator {
    virtual ~GenericAllocator() = default;
    virtual auto alloc(std::size_t s) -> void * = 0;
    virtual auto dealloc(void * p) -> void = 0;
    virtual auto realloc(void * p, std::size_t s) -> void * = 0;
};

template <typename Allocator> class AllocatorAdaptator : public GenericAllocator {
  public:
    auto alloc(std::size_t s) -> void * override { return _allocator.alloc(s); }
    auto dealloc(void * p) -> void override { return _allocator.dealloc(p); }
    auto realloc(void * p, std::size_t s) -> void * override { return _allocator.realloc(p, s); }

  private:
    Allocator & _allocator;
};

struct StdAllocator : public GenericAllocator {
    auto alloc(std::size_t s) -> void * override { return std::malloc(s); }
    auto dealloc(void * p) -> void override { std::free(p); }
    auto realloc(void * p, std::size_t s) -> void * override { return std::realloc(p, s); }
};

inline StdAllocator std_allocator;

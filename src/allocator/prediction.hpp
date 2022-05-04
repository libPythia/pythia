#pragma once

#include <eta/factorization/reduction.hpp>
#include <iosfwd>

#include "allocator.hpp"

enum class Operation {
    Allocation,
    Deallocation,
    Reallocation,
};

struct Event {
    Operation operation;
    size_t orig;
    size_t dest;
};

auto serialize(Terminal const * t, std::ostream &) -> void;
auto deserialize(std::istream &, size_t size) -> void *;


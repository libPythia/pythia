#pragma once

#include <cstddef>

struct Probability {
    size_t count;
    size_t total;

    auto as_double() const { return static_cast<double>(count) / static_cast<double>(total); };
};


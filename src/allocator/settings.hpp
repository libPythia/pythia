#pragma once

enum class Mode {
    Disabled,
    Recording,
    Predicting,
};

struct Settings final {
    bool record_allocations;
    char const * allocations_file;

    Mode mode;
    char const * trace_file;
    char const * extension;
};

auto get_settings() -> Settings const *;


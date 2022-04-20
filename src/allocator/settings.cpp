#include "settings.hpp"

#include <stdlib.h>
#include <string.h>

#include "debug.hpp"

auto get_settings() -> Settings {
    log_fn;
    auto const record_allocations_opt = getenv("PYTHIA_RECORD_MEMORY");
    auto const memory_record_file_opt = getenv("PYTHIA_MEMORY_LOG_FILE");
    auto const mode_opt = getenv("PYTHIA_ALLOC_MODE");
    auto const trace_file = getenv("PYTHIA_ALLOC_TRACE_FILE");

    auto settings = Settings {};

    settings.record_allocations =
            record_allocations_opt != nullptr && strcmp(record_allocations_opt, "YES") == 0;

    settings.allocations_file = [&]() -> char const * {
        if (memory_record_file_opt == nullptr)
            return "allocations.log";
        return memory_record_file_opt;
    }();

    settings.mode = [&]() {
        if (mode_opt == nullptr)
            return Mode::Disabled;
        if (strcmp(mode_opt, "Record") == 0)
            return Mode::Recording;
        if (strcmp(mode_opt, "Predict") == 0)
            return Mode::Predicting;
        return Mode::Disabled;
    }();

    settings.trace_file = [&]() -> char const * {
        if (trace_file == nullptr)
            return "pythia_allocator";
        return trace_file;
    }();

    settings.extension = ".btr";

    return settings;
}


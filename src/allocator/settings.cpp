#include "settings.hpp"

#include <stdlib.h>
#include <string.h>

#include "debug.hpp"

static char * allocations_file;
static char * trace_file;

auto get_settings() -> Settings const * {
    log_fn;
    static auto const res = []() -> Settings {
        auto settings = Settings {};

        // Record allocation or not
        auto const record_allocations_opt = getenv("PYTHIA_RECORD_MEMORY");
        settings.record_allocations =
                record_allocations_opt != nullptr && strcmp(record_allocations_opt, "YES") == 0;

        // Log file where to export recorded allocations
        auto const memory_record_file_opt = getenv("PYTHIA_MEMORY_LOG_FILE");
        if (memory_record_file_opt == nullptr) {
            auto constexpr default_path = "allocations.log";
            allocations_file = static_cast<char *>(malloc(strlen(default_path) + 1));
            strcpy(allocations_file, default_path);
        } else {
            allocations_file = static_cast<char *>(malloc(strlen("allocations.log") + 1));
            strcpy(allocations_file, memory_record_file_opt);
        }
        settings.allocations_file = allocations_file;

        // Allocator mode : record, predict or nothing ?
        auto const mode_opt = getenv("PYTHIA_ALLOC_MODE");
        settings.mode = [&]() {
            if (mode_opt == nullptr)
                return Mode::Disabled;
            if (strcmp(mode_opt, "Record") == 0)
                return Mode::Recording;
            if (strcmp(mode_opt, "Predict") == 0)
                return Mode::Predicting;
            return Mode::Disabled;
        }();

        auto const trace_file_opt = getenv("PYTHIA_ALLOC_TRACE_FILE");
        if (trace_file_opt == nullptr) {
            auto constexpr default_path = "pythia_allocator";
            auto const size = strlen(default_path);
            trace_file = static_cast<char *>(malloc(size + 1));
            strcpy(trace_file, default_path);
        } else {
            auto const size = strlen(trace_file_opt);
            trace_file = static_cast<char *>(malloc(size + 1));
            strcpy(trace_file, trace_file_opt);
        }
        settings.trace_file = trace_file;

        settings.extension = ".btr";

        return settings;
    }();
    return &res;
}

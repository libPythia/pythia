#pragma once

enum class errors_t : char {
    NO_ERROR = 0,
    BAD_ARGUMENTS,
    INPUT_OUTPUT_MISMATCH,
    CHECK_FAILED,
    NOT_IMPLEMENTED_FEATURE,
};

inline auto exit(errors_t err) { exit(static_cast<int>(err)); }

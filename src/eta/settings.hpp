#pragma once

enum class input_t {
    lines,
    text,
    non_printable,
    binary,
};

enum class output_t {
    reduced,
    dot,
    grammar,
    binary,
};

struct settings_t final {
    // general
    bool debug;
    bool check;

    // input
    input_t input_mode;

    // output
    output_t output_mode;
    bool print_input;
};

auto parse_settings(int argc, char ** argv) -> settings_t;


#pragma once

#include <string>
#include <vector>

enum class input_src_t {
    file,
    std_in,
    argument,
};

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
    expend,
    terminals,
};

struct settings_t final {
    // general
    bool debug;
    bool check;
    bool replay;
    std::string compare;

    // input
    input_src_t input_src;
    input_t input_mode;
    std::vector<std::string> input_data;

    // output
    output_t output_mode;
    bool print_input;
};

auto parse_settings(int argc, char ** argv) -> settings_t;


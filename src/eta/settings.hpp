#pragma once

#include <string>
#include <vector>

enum class input_src_t {
    file,
    std_in,
    argument,
};

enum class input_t {
    logs,
    lines,
    text,
    non_printable,
    binary,
};

enum class output_t {
    reduced,
    dot,
    flow,
    grammar,
    binary,
    expend,
    terminals,
    tree,
#ifdef ETA_GUI_ENABLED
    gui,
#endif
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


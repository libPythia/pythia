#pragma once

#include <iostream>
#include <memory>

#include "colors.hpp"
#include "data.hpp"
#include "settings.hpp"

inline auto print_error(char const * txt) -> void {
    set_color(std::cerr, color_t::red);
    std::cerr << "error: " << txt << std::endl;
    set_color(std::cerr, color_t::standard);
}

template <typename... T>
inline auto print_debug(settings_t const & settings, T &&... args) -> void {
    if (settings.debug) {
        set_color(std::cerr, color_t::yellow);
        std::cerr << "debug: ";
        ((std::cerr << args), ...);
        set_color(std::cerr, color_t::standard);
        std::cerr << std::endl;
    }
}

enum class input_state {
    none,
    last,
    yes,
};

class Input {
    struct Impl;

  public:
    Input(settings_t const & settings);
    ~Input();

    auto read_input(Grammar & g) -> input_state;

  private:
    std::unique_ptr<Impl> _impl;
};


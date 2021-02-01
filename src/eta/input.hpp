#pragma once

#include <iostream>
#include <vector>

#include "colors.hpp"
#include "data.hpp"
#include "settings.hpp"

inline auto print_error(char const * txt) -> void {
    set_color(std::cerr, color_t::red);
    set_style(std::cerr, style_t::bold);
    std::cerr << "error: ";
    set_color(std::cerr, color_t::standard);
    std::cerr << txt << std::endl;
    set_style(std::cerr, style_t::standard);
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

struct Input {
    Grammar grammar;
    std::vector<NonTerminal *> threads;
};

auto read_input(settings_t const & settings) -> Input;


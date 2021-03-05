#pragma once

#include <eta/core/colors.hpp>
#include <iostream>
#include <vector>

#include "data.hpp"
#include "settings.hpp"

inline auto print_error(char const * txt) -> void {
    eta::set_color(std::cerr, eta::color_t::red);
    eta::set_style(std::cerr, eta::style_t::bold);
    std::cerr << "error: ";
    eta::set_color(std::cerr, eta::color_t::standard);
    std::cerr << txt << std::endl;
    eta::set_style(std::cerr, eta::style_t::standard);
}

template <typename... T>
inline auto print_debug(settings_t const & settings, T &&... args) -> void {
    if (settings.debug) {
        eta::set_color(std::cerr, eta::color_t::yellow);
        std::cerr << "debug: ";
        ((std::cerr << args), ...);
        eta::set_color(std::cerr, eta::color_t::standard);
        std::cerr << std::endl;
    }
}

struct Input {
    Grammar grammar;
    std::vector<NonTerminal *> threads;
};

auto read_input(settings_t const & settings) -> Input;


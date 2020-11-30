#pragma once

#include <ostream>

enum class color_t {
    blue,
    cyan,
    gray,
    green,
    magenta,
    red,
    standard,
    yellow,
};

enum class style_t {
    bold,
    italic,
    reversed,
    standard,
    underline,
};

auto set_style(std::ostream & os, style_t style) -> void;
auto set_color(std::ostream & os, color_t color) -> void;


#include "colors.hpp"

#include <ostream>
#include <rang.hpp>

auto set_color(std::ostream & os, color_t color) -> void {
    switch (color) {
        case color_t::blue: os << rang::fg::blue; break;
        case color_t::cyan: os << rang::fg::cyan; break;
        case color_t::gray: os << rang::fg::gray; break;
        case color_t::green: os << rang::fg::green; break;
        case color_t::magenta: os << rang::fg::magenta; break;
        case color_t::red: os << rang::fg::red; break;
        case color_t::standard: os << rang::fg::reset; break;
        case color_t::yellow: os << rang::fg::yellow; break;
    }
}

auto set_style(std::ostream & os, style_t style) -> void {
    switch (style) {
        case style_t::bold: os << rang::style::bold; break;
        case style_t::italic: os << rang::style::italic; break;
        case style_t::reversed: os << rang::style::reversed; break;
        case style_t::standard: os << rang::style::reset; break;
        case style_t::underline: os << rang::style::underline; break;
    }
}


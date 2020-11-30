#pragma once

#include <functional>
#include <ostream>

#include "factorization.hpp"

using terminal_printer = std::function<void(Terminal const *, std::ostream &)>;

auto linearise_grammar(Grammar const & g) -> std::vector<Terminal const *>;
auto print_trace(Grammar const &, std::ostream &, terminal_printer const &) -> void;
auto print_reduced_trace(Grammar const &, std::ostream &, terminal_printer const &) -> void;
auto print_grammar(Grammar const &, std::ostream &, terminal_printer const &) -> void;

auto print_dot_file_begin(std::ostream &) -> void;
auto print_dot_file_grammar(Grammar const &,
                            std::ostream &,
                            terminal_printer const &,
                            bool print_input) -> void;
auto print_dot_file_end(std::ostream &) -> void;
auto print_dot_file(Grammar const &, std::ostream &, terminal_printer const &, bool print_input)
        -> void;

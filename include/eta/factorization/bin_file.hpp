#include <functional>
#include <istream>
#include <ostream>

#include "reduction.hpp"

using terminal_printer = std::function<void(Terminal const *, std::ostream &)>;
using terminal_reader = std::function<void *(std::istream &, size_t)>;

auto print_bin_file(Grammar const &, std::ostream &, terminal_printer const &) -> void;
auto load_bin_file(Grammar &, std::istream &) -> void;
auto load_bin_file(Grammar &, std::istream &, terminal_reader const &) -> void;


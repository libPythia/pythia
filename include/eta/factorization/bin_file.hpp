#include <functional>
#include <istream>
#include <ostream>
#include <string>
#include <unordered_map>

#include "factorization.hpp"

using terminal_printer = std::function<void(Terminal const *, std::ostream &)>;

auto print_bin_file(Grammar const &, std::ostream &, terminal_printer const &) -> void;
auto load_bin_file(Grammar &, std::istream &) -> std::unordered_map<Terminal const *, std::string>;


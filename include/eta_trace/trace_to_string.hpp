#pragma once

#include <functional>
#include <string>

#include "trace.hpp"

auto linearise(Node const * node) -> std::vector<LeafId>;
auto toStr(Node const * node, bool show_parenthesis, std::function<std::string(LeafId)>) -> std::string;

#pragma once

#include <functional>
#include <ostream>
#include <string>

#include "trace.hpp"

namespace eta::factorization {

auto linearise(Node const * node) -> std::vector<LeafId>;

auto toStr(Node const * node, bool show_parenthesis, std::function<std::string(LeafId)>)
        -> std::string;

auto writeDotFile(std::ostream & os,
                  Node const * start,
                  std::function<std::string(Node const *)> getLabel,
                  std::function<std::string(LeafId)> getLeaf) -> void;

}  // namespace eta::factorization


#pragma once

#include <functional>
#include <ostream>
#include <string>

#include "trace.hpp"

auto writeDotFile(std::ostream & os,
                  Node const * start,
                  std::function<std::string(Node const *)> getLabel,
                  std::function<std::string(LeafId)> getLeaf) -> void;


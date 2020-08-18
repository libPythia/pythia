#pragma once
#include <tuple>

#include "trace.hpp"

namespace eta::factorization {

auto insertNode(Trace & trace, Node * next, Node * last) -> Node *;

class TraceBuilder final {
  public:
    TraceBuilder(allocator_t & allocator);
    auto newLeaf() { return _trace.newLeaf(); }
    auto createMultipleLeafs(std::size_t count) -> std::pair<LeafId, LeafId>;
    auto insert(LeafId leaf_id) -> void;
    auto trace() const -> Trace const & { return _trace; }
    auto trace() -> Trace & { return _trace; }

  private:
    Trace _trace;
    Node * _last = nullptr;
};

}  // namespace eta::factorization

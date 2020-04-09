#pragma once

#include "trace.hpp"

auto insertNode(Trace & trace, Node * next, Node * last) -> Node *;

class TraceBuilder final {
  public:
    TraceBuilder(GenericAllocator & allocator);
    auto newLeaf() { return _trace.newLeaf(); }
    auto insert(LeafId leaf_id) -> void;
    auto trace() const -> Trace const & { return _trace; }

  private:
    Trace _trace;
    Node * _last = nullptr;
};

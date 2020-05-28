#pragma once

#include "eta/core/chunk_vector.hpp"
#include "eta/core/vector.hpp"
#include "node.hpp"

// -----------------------------------------------------------

namespace eta {

class Trace final {
  public:
    Trace(allocator_t & alloc);
    ~Trace();

  public:
    auto operator[](std::size_t index) const -> Node const & { return _nodes[index]; }
    auto operator[](LeafId id) const -> Node * { return _leafs[id.value()]; }
    auto nodeCount() const { return _nodes.size(); }

    auto newLeaf() -> LeafId;
    auto setLeaf(LeafId id, Node * node) -> void;

    auto newNode() -> Node *;
    auto releaseNode(Node * pNode) -> void;

    auto allocator() -> allocator_t & { return _allocator; }

    auto root() const -> Node const * { return _root; }
    auto root() -> Node * { return _root; }
    auto setRoot(Node * pNode) -> void;

  private:
    chunk_vector<Node, 128> _nodes;
    vector<Node *> _free_nodes;
    vector<Node *> _leafs;
    allocator_t & _allocator;
    Node * _root = nullptr;
};

// -----------------------------------------------------------

auto computeIndicesAndOffset(allocator_t & allocator, Trace & trace) -> void;

// -----------------------------------------------------------

}  // namespace eta

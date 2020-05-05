#pragma once

#include "eta/core/chunk_vector.hpp"
#include "eta/core/vector.hpp"

#include "node.hpp"

// -----------------------------------------------------------

class Trace final {
  public:
    Trace(GenericAllocator & alloc);
    ~Trace();

  public:
    auto operator[](std::size_t index) const -> Node const & { return _nodes[index]; }
    auto operator[](LeafId id) const -> Node * { return _leafs[id.value()]; }
    auto nodeCount() const { return _nodes.size(); }

    auto newLeaf() -> LeafId;
    auto setLeaf(LeafId id, Node * node) -> void;

    auto newNode() -> Node *;
    auto releaseNode(Node * pNode) -> void;

    auto allocator() -> GenericAllocator & { return _allocator; }

    auto root() const -> Node const * { return _root; }
    auto root() -> Node * { return _root; }
    auto setRoot(Node * pNode) -> void;

  private:
    chunk_vector<Node, 128> _nodes;
    vector<Node *> _free_nodes;
    vector<Node *> _leafs;
    GenericAllocator & _allocator;
    Node * _root = nullptr;
};

// -----------------------------------------------------------

auto computeIndicesAndOffset(GenericAllocator & allocator, Trace & trace) -> void;

// -----------------------------------------------------------


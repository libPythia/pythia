#pragma once

#include "eta/core/chunk_vector.hpp"
#include "eta/core/vector.hpp"

#include "node.hpp"

// -----------------------------------------------------------

class Trace final {
  public:
    auto operator[](std::size_t index) const -> Node const & { return _nodes[index]; }
    auto operator[](LeafId id) const -> Node * { return _leafs[id.value()]; }

    auto setLeaf(LeafId id, Node * node) -> void { _leafs[id.value()] = node; }

    auto releaseNode(Node * pNode) -> void {
        assert(pNode->parents.size() == 0u);
        assert(pNode->next == nullptr);
        assert(pNode->previous == nullptr);
        assert(!pNode->isLeaf());
        assert(pNode->son() == nullptr);
        _free_nodes.push_back(_allocator, pNode);
    }

    auto allocator() -> GenericAllocator & { return _allocator; }

    auto newNode() -> Node * {
        if (_free_nodes.size() > 0) {
            auto const res = _free_nodes.back();
            _free_nodes.pop_back();
            return res;
        } else {
            _nodes.push_back(_allocator);
            return &_nodes.back();
        }
    }

    auto newLeaf() -> LeafId {
        auto const node = newNode();
        auto const leaf_id = LeafId(_leafs.size());
        _leafs.push_back(_allocator, node);
        node->setLeaf(leaf_id);
        return leaf_id;
    }

    auto root() const -> Node const * { return _root; }
    auto setRoot(Node * pNode) {
        assert(_root == nullptr);
        assert(pNode != nullptr);
        _root = pNode;
    }

    auto nodeCount() const { return _nodes.size(); }

  public:
    Trace(GenericAllocator & alloc)
          : _allocator { alloc } {}
    ~Trace() {
        auto const node_count = _nodes.size();
        for (auto i = 0u; i < node_count; ++i)
            _nodes[i].parents.deinit(_allocator);
        _nodes.deinit(_allocator);
        _free_nodes.deinit(_allocator);
        _leafs.deinit(_allocator);
    }

  private:
    chunk_vector<Node, 128> _nodes;
    vector<Node *> _free_nodes;
    vector<Node *> _leafs;
    GenericAllocator & _allocator;
    Node * _root = nullptr;
};

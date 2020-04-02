#pragma once

#include <cstdint>
#include <tuple>

#include "eta/core/chunk_vector.hpp"
#include "eta/core/vector.hpp"
#include "impl/parents_tree.hpp"

// -----------------------------------------------------------

struct LeafId {
  public:
    using underlying_t = unsigned int;

  public:
    constexpr LeafId(underlying_t v)
          : _value(v) {}

    auto value() const { return _value; }

    auto operator<(LeafId o) const { return _value < o._value; }
    auto operator==(LeafId o) const { return _value == o._value; }
    auto operator!=(LeafId o) const { return _value != o._value; }

  private:
    underlying_t _value;
};

// -----------------------------------------------------------

struct keep_previous_t {};
static constexpr keep_previous_t keep_previous;

class Node final {
  public:
    Node()
          : parents()
          , previous(nullptr)
          , next(nullptr)
          , _loop(1u) {
        _son = nullptr;
    }
    Node(Node &&) = delete;
    Node(Node const &) = delete;
    auto operator=(Node &&) -> Node & = delete;
    auto operator=(Node const &) -> Node & = delete;

  public:
    Parents parents;
    Node * previous;
    Node * next;

  public:
    auto isLeaf() const -> bool { return _loop == 0u; }
    auto son() const -> Node * {
        if (isLeaf())
            return nullptr;
        return _son;
    }

    auto value() const -> LeafId {
        assert(_loop == 0u);
        return _value;
    }

    auto setSon(Node * node, std::size_t loop) -> void {
        assert(loop > 0u);
        _son = node;
        _loop = loop;
    }

    auto setSon(Node * node, keep_previous_t) -> void {
        assert(node != nullptr);
        _son = node;
    }

    auto setSon(std::nullptr_t) -> void {
        _son = nullptr;
        _loop = 1u;
    }

    auto setLeaf(LeafId id) -> void {
        _value = id;
        _loop = 0u;
    }

    auto setLoop(std::uint32_t v) -> void {
        assert(_loop != 0u);
        assert(v > 0u);
        _loop = v;
    }

    auto loop() const {
        if (_loop == 0u)
            return 1u;
        return _loop;
    }

  private:
    union {
        Node * _son;
        LeafId _value;
    };
    std::uint32_t _loop;
};

// -----------------------------------------------------------

template <typename Allocator> class Trace final {
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

    auto allocator() -> Allocator & { return _allocator; }

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
    Allocator _allocator;
    Node * _root = nullptr;
};

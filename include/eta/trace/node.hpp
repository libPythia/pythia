#pragma once

#include "leaf_id.hpp"
#include "parents_tree.hpp"

// -----------------------------------------------------------

struct keep_previous_t {};
static constexpr keep_previous_t keep_previous;

// -----------------------------------------------------------

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

    auto setLoop(unsigned int v) -> void {
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
    unsigned int _loop;
};

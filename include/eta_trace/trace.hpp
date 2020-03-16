#include <cstdint>
#include <tuple>

#include "chunk_vector.hpp"
#include "ids.hpp"
#include "parents_tree.hpp"
#include "vector.hpp"

// -----------------------------------------------------------

class Node final {
  public:
    Node()
          : parents()
          , previous(NodeId::invalid())
          , next(NodeId::invalid())
          , _loop(1u) {
        _son = NodeId::invalid();
    }
    Node(Node &&) = delete;
    Node(Node const &) = delete;
    auto operator=(Node &&) -> Node & = delete;
    auto operator=(Node const &) -> Node & = delete;

  public:
    Parents parents;
    NodeId previous;
    NodeId next;

  public:
    auto isLeaf() const -> bool { return _loop == 0u; }
    auto son() const -> NodeId {
        if (isLeaf())
            return NodeId::invalid();
        return _son;
    }

    auto value() const -> LeafId {
        assert(_loop == 0u);
        return _value;
    }

    auto setSon(NodeId id) -> void {
        _son = id;
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
        NodeId _son;
        LeafId _value;
    };
    std::uint32_t _loop;
};

// -----------------------------------------------------------

template <typename Allocator> class Trace final {
  public:
    auto operator[](NodeId id) -> Node & {
        assert(id.isValid());
        return _nodes[id.value()];
    }

    auto operator[](NodeId id) const -> Node const & {
        assert(id.isValid());
        return _nodes[id.value()];
    }

    auto operator[](LeafId id) const -> NodeId { return _leafs[id.value()]; }

    auto setLeaf(LeafId leaf_id, NodeId node_id) -> void { _leafs[leaf_id.value()] = node_id; }

    auto releaseNode(NodeId id) -> void {
        assert(_nodes[id.value()].parents.size() == 0u);
        assert(!_nodes[id.value()].next.isValid());
        assert(!_nodes[id.value()].previous.isValid());
        assert(!_nodes[id.value()].isLeaf());
        assert(!_nodes[id.value()].son().isValid());
        _free_nodes.push_back(_allocator, id);
    }

    auto allocator() -> Allocator & { return _allocator; }

    auto newNode() -> NodeId {
        if (_free_nodes.size() > 0) {
            auto const res = _free_nodes.back();
            _free_nodes.pop_back();
            return res;
        } else {
            auto const res = NodeId(_nodes.size());
            _nodes.push_back(_allocator);
            return res;
        }
    }

    auto newLeaf() -> LeafId {
        auto const node_id = newNode();
        auto const leaf_id = LeafId(_leafs.size());
        _leafs.push_back(_allocator, node_id);
        operator[](node_id).setLeaf(leaf_id);
        return leaf_id;
    }

    auto root() const { return _root; }
    auto setRoot(NodeId id) {
        assert(!_root.isValid());
        assert(id.isValid());
        _root = id;
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
    vector<NodeId> _free_nodes;
    vector<NodeId> _leafs;
    Allocator _allocator;
    NodeId _root = NodeId::invalid();
};


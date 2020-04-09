#include "trace.hpp"

// -----------------------------------------------------------

Trace::Trace(GenericAllocator & alloc)
      : _allocator { alloc } {}

Trace::~Trace() {
    auto const node_count = _nodes.size();
    for (auto i = 0u; i < node_count; ++i)
        _nodes[i].parents.deinit(_allocator);
    _nodes.deinit(_allocator);
    _free_nodes.deinit(_allocator);
    _leafs.deinit(_allocator);
}

// -----------------------------------------------------------

auto Trace::newLeaf() -> LeafId {
    auto const node = newNode();
    auto const leaf_id = LeafId(_leafs.size());
    _leafs.push_back(_allocator, node);
    node->setLeaf(leaf_id);
    return leaf_id;
}

auto Trace::setLeaf(LeafId id, Node * node) -> void {
    assert(node != nullptr);
    assert(node->isLeaf());
    assert(node->value() == id);
    _leafs[id.value()] = node;
}

// -----------------------------------------------------------

auto Trace::newNode() -> Node * {
    if (_free_nodes.size() > 0) {
        auto const res = _free_nodes.back();
        _free_nodes.pop_back();
        return res;
    } else {
        _nodes.push_back(_allocator);
        return &_nodes.back();
    }
}

auto Trace::releaseNode(Node * pNode) -> void {
    assert(pNode->parents.size() == 0u);
    assert(pNode->next == nullptr);
    assert(pNode->previous == nullptr);
    assert(!pNode->isLeaf());
    assert(pNode->son() == nullptr);
    _free_nodes.push_back(_allocator, pNode);
}

// -----------------------------------------------------------

auto Trace::setRoot(Node * pNode) -> void {
    assert(_root == nullptr);
    assert(pNode != nullptr);
    _root = pNode;
}

// -----------------------------------------------------------


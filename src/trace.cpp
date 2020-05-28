#include "trace.hpp"

namespace eta {

// -----------------------------------------------------------

Trace::Trace(allocator_t & alloc) : _allocator { alloc } {}

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
    node->size = 1;
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

auto computeIndicesAndOffset(allocator_t & allocator, Trace & trace) -> void {
    auto buffer = vector<Node *> {};
    for (auto it = trace.root(); it != nullptr; it = it->next)
        buffer.push_back(allocator, it);

    while (buffer.size() > 0) {
        auto node = buffer.back();
        if (node->size >= 0)
            buffer.pop_back();
        else if (node->son()->size < 0) {
            for (auto it = node->son(); it != nullptr; it = it->next) {
                assert(it->size == -1);
                buffer.push_back(allocator, it);
            }
        } else {
            auto it = node->son();
            it->offset = 0;
            node->size = it->size * it->loop();

            while ((it = it->next) != nullptr) {
                node->size += it->size * it->loop();
                it->offset = it->previous->offset + it->previous->size * it->previous->loop();
            }
            buffer.pop_back();
        }
    }

    auto it = trace.root();
    it->offset = 0;
    while ((it = it->next) != nullptr)
        it->offset = it->previous->offset + it->previous->size * it->previous->loop();

    buffer.deinit(allocator);
}

// -----------------------------------------------------------

}  // namespace eta

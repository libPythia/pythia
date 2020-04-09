#pragma once

#include "eta_trace/trace.hpp"

inline auto sonId(Node * node) -> Node * { return node == nullptr ? nullptr : node->son(); }

inline auto updatePreviousSon(Node * previous, Node * old_son, Node * new_son) -> void {
    if (previous == nullptr)
        return;
    assert(old_son != new_son);
    auto const previous_son = previous->son();
    assert(previous_son != nullptr);

    previous_son->parents.replace(previous, old_son, new_son);
}

inline auto setNext(Node * node, Node * next) -> void {
    assert(node != nullptr);
    assert(next != nullptr);
    assert(node != next);
    assert(node->next == nullptr);
    assert(next->previous == nullptr);

    auto const son = node->son();
    assert(son != nullptr);

    son->parents.replace(node, nullptr, next->son());
    node->next = next;
    next->previous = node;
}

inline auto setSon(GenericAllocator & alloc, Node * node, Node * son, std::size_t loop) -> void {
    assert(node != nullptr);
    assert(son != nullptr);
    assert(node != son);
    assert(node->son() == nullptr);
    assert(son->previous == nullptr);
    assert(loop > 0u);
    node->setSon(son, loop);
    updatePreviousSon(node->previous, nullptr, son);
    son->parents.insert(alloc, node, sonId(node->next));
}

inline auto transferValueOrSon(Trace & trace, Node * from, Node * to) -> void {
    assert(from != nullptr);
    assert(to != nullptr);
    assert(from != to);
    assert(!to->isLeaf() && to->son() == nullptr);

    if (from->isLeaf()) {
        auto const leaf_id = from->value();
        to->setLeaf(leaf_id);
        trace.setLeaf(leaf_id, to);
        from->setSon(nullptr);
    } else {
        auto const son = from->son();
        assert(son != nullptr);
        assert(son != to);
        assert(son->previous == nullptr);
        updatePreviousSon(from->previous, son, nullptr);
        updatePreviousSon(to->previous, nullptr, son);
        son->parents.remove(from, sonId(from->next));
        son->parents.insert(trace.allocator(), to, sonId(to->next));
        to->setSon(from->son(), from->loop());
        from->setSon(nullptr);
    }
}

inline auto transferNext(Node * from, Node * to) -> void {
    assert(from != nullptr);
    assert(to != nullptr);
    assert(to->next == nullptr);

    if (from->next != nullptr) {
        auto const from_son = from->son();
        auto const to_son = to->son();

        if (from_son != nullptr)
            from_son->parents.replace(from, from->next->son(), nullptr);

        if (to_son != nullptr)
            to_son->parents.replace(to, nullptr, from->next->son());

        from->next->previous = to;
        to->next = from->next;
        from->next = nullptr;
    }
}

inline auto transferNext(Node * n1, Node * n2, Node * n3) -> void {
    transferNext(n2, n3);
    transferNext(n1, n2);
}

inline auto removeNext(Node * from) -> void {
    assert(from != nullptr);
    assert(from->next != nullptr);

    auto const son = from->son();
    if (son != nullptr)
        son->parents.replace(from, sonId(from->next), nullptr);

    from->next->previous = nullptr;
    from->next = nullptr;
}

inline auto removeSon(Node * parent) -> void {
    auto son = parent->son();
    assert(son != nullptr);
    assert(son->previous == nullptr);
    updatePreviousSon(parent->previous, son, nullptr);
    son->parents.remove(parent, sonId(parent->next));
    parent->setSon(nullptr);
}

inline auto changeNext(Node * node, Node * new_next) -> void {
    assert(node != nullptr);
    assert(new_next != nullptr);
    assert(new_next->previous == nullptr);

    auto const old_next = node->next;
    assert(old_next != nullptr);

    auto const son = node->son();
    if (son != nullptr)
        son->parents.replace(node, old_next, new_next);

    node->next = new_next;
    old_next->previous = nullptr;
    new_next->previous = node;
}

inline auto changeSon(GenericAllocator & alloc, Node * parent, Node * new_son) -> void {
    assert(parent != nullptr);
    assert(new_son != nullptr);
    assert(parent->son()->previous == nullptr);
    assert(new_son->previous == nullptr);
    assert(parent != new_son);

    auto const old_son = parent->son();
    auto const next_son = sonId(parent->next);

    old_son->parents.remove(parent, next_son);
    new_son->parents.insert(alloc, parent, next_son);
    updatePreviousSon(parent->previous, old_son, new_son);
    parent->setSon(new_son, keep_previous);
}

inline auto detachAndReleaseNode(Trace & trace, Node * node) -> void {
    assert(node != nullptr);
    if (node->previous != nullptr)
        removeNext(node->previous);
    if (node->next != nullptr)
        removeNext(node);
    auto const son = node->son();
    if (son != nullptr)
        removeSon(node);
    trace.releaseNode(node);
}

inline auto appendOccurence(Trace & trace, Node * last, Node * son) -> Node * {
    assert(last != nullptr);
    assert(son != nullptr);
    assert(last->next == nullptr);
    assert(last->son() != nullptr);
    assert(son->previous == nullptr);

    auto const node = trace.newNode();

    son->parents.insert(trace.allocator(), node, nullptr);
    node->setSon(son, 1u);

    last->son()->parents.replace(last, nullptr, son);
    last->next = node;
    node->previous = last;

    return node;
}

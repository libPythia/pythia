#pragma once
#include <iostream>

#include "impl/trace_edition.hpp"

auto searchParentPattern(Node * last, Node * next) -> Node *;
auto getLastPatternNode(Node * node) -> Node *;

#ifndef ETA_FACTO_PRINT
#    define ETA_FACTO_PRINT(args)
#endif
// std::cerr << ((args))

enum Side {
    RightSide,
    LeftSide,
};

template <Side side, typename Allocator>
auto splitLoop(Trace<Allocator> & trace, Node * loop, std::size_t count) -> Node * {
    assert(loop != nullptr);
    assert(count > 0u);
    assert(!loop->isLeaf());

    auto const loop_count = loop->loop();
    assert(loop_count > count);

    auto const node = trace.newNode();
    if (loop->next != nullptr)
        transferNext(loop, node);
    setNext(loop, node);

    if constexpr (side == LeftSide) {
        loop->setLoop(count);
        setSon(trace.allocator(), node, loop->son(), loop_count - count);
        return loop;
    } else {
        loop->setLoop(loop_count - count);
        setSon(trace.allocator(), node, loop->son(), count);
        return node;
    }
}

template <typename Allocator>
auto insertNode(Trace<Allocator> & trace, Node * next, Node * last) -> Node * {
    assert(next != nullptr);
    assert(last != nullptr);
    assert(last->next == nullptr);
    assert(!last->isLeaf());

    ////////////////////////////////
    // short exit : factorize loop

    if (last->son() == next) {
        ETA_FACTO_PRINT("Loop");
        last->setLoop(last->loop() + 1);
        return last;
    }

    //////////////////////////////////////
    // short exit : nothing to factorize

    auto parent = searchParentPattern(last, next);
    if (parent == nullptr) {
        ETA_FACTO_PRINT("Append");
        return appendOccurence(trace, last, next);
    }

    //////////////////////////
    // split loops if needed

    assert(last->previous != nullptr);

    // if (parent->loop() > 1)
    //     parent = splitLoop<RightSide>(trace, parent, 1);
    // if (last->loop() > 1)
    //     last = splitLoop<RightSide>(trace, last, 1);
    if (parent->next->loop() > 1)
        splitLoop<LeftSide>(trace, parent->next, 1);

    auto const parent_loop = parent->loop();
    auto const last_loop = last->loop();
    if (last_loop > parent_loop)
        last = splitLoop<RightSide>(trace, last, parent_loop);
    else if (parent_loop > last_loop)
        parent = splitLoop<RightSide>(trace, parent, last_loop);
    assert(parent->loop() == last->loop());

    auto const parent_son = parent->son();
    auto const previous = last->previous;

    ///////////////////////////////////////
    // Can an existing pattern be reused?

    // Can an existing pattern be extended ?
    if (!parent_son->isLeaf() &&  // pattern to extend
        parent_son->parents.size() <= 2 &&  // two parents : parent and last
        parent->loop() == 1u && last->loop() == 1) {  // cannot extend the pattern if it
                                                      // repeats

        auto const last_pattern_node = getLastPatternNode(parent_son);
        auto const parent_next = parent->next;

        removeNext(previous);
        removeSon(last);
        trace.releaseNode(last);

        removeNext(parent);
        removeSon(parent_next);
        if (parent->parents.size() > 0u && parent_next->next == nullptr) {
            ETA_FACTO_PRINT("Merge");
            trace.releaseNode(parent_next);
            removeSon(parent);
            transferNext(parent_son, parent);
            transferValueOrSon(trace, parent_son, parent);
            trace.releaseNode(parent_son);
            insertNode(trace, next, last_pattern_node);
            return insertNode(trace, parent, previous == parent_next ? parent : previous);
        } else {
            ETA_FACTO_PRINT("Extend");
            if (parent_next != nullptr)
                transferNext(parent_next, parent);
            trace.releaseNode(parent_next);

            insertNode(trace, next, last_pattern_node);
            return insertNode(trace, parent_son, previous == parent_next ? parent : previous);
        }
    }

    if (parent->previous == nullptr && parent->next->next == nullptr) {
        ETA_FACTO_PRINT("Reuse");
        removeNext(previous);
        removeSon(last);
        trace.releaseNode(last);
        return insertNode(trace, parent, previous);
    }

    /////////////////////////
    // Create a new pattern

    ETA_FACTO_PRINT("Create");
    // last will be reused for the new pattern
    removeNext(previous);
    removeSon(last);

    transferValueOrSon(trace, parent, last);
    transferNext(parent, last);
    if (last->next->next != nullptr)  // TODO useful ?
        transferNext(last->next, parent);

    setSon(trace.allocator(), parent, last, 1u);
    return insertNode(trace, last, previous == last->next ? parent : previous);
}

template <typename Allocator> class TraceBuilder final {
  public:
    auto newLeaf() { return _trace.newLeaf(); }
    auto insert(LeafId leaf_id) -> void {
        if (_last != nullptr) {
            auto const node_id = _trace[leaf_id];
            _last = insertNode(_trace, node_id, _last);
        } else {
            ETA_FACTO_PRINT("Init");
            _last = _trace.newNode();
            setSon(_trace.allocator(), _last, _trace[leaf_id], 1u);
            _trace.setRoot(_last);
        }
    }

    auto trace() const -> Trace<Allocator> const & { return _trace; }

  private:
    Trace<Allocator> _trace;
    Node * _last = nullptr;
};

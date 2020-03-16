#pragma once

#include "trace_edition.hpp"

template <typename Allocator>
auto searchParentPattern(Trace<Allocator> const & trace,
                         NodeId last_id,
                         NodeId next_id,
                         NodeId excepted = NodeId::invalid()) -> NodeId {
    auto const last_son_id = trace[last_id].son();

    if (last_son_id.isValid()) {
        auto const parent_id = trace[last_son_id].parents[next_id];
        if (parent_id.isValid()) {
            auto const & parent = trace[parent_id];
            if (parent.next.isValid()) {
                if (trace[parent.next].son() == next_id &&
                    (!excepted.isValid() || parent.next != excepted)) {
                    return parent_id;
                }
            }
        }
    }

    return NodeId::invalid();
}

// No need of isTrivialNode and simplifyTrivialNode ?

template <typename Allocator>
auto getLastPatternNode(Trace<Allocator> const & trace, NodeId id) -> NodeId {
    auto last_id = NodeId::invalid();

    while (id.isValid()) {
        last_id = id;
        id = trace[id].next;
    }
    return last_id;
}

template <typename Allocator>
auto factorizeLoop(Trace<Allocator> & trace, NodeId last_id) -> NodeId {
    auto & last = trace[last_id];
    auto const previous_id = last.previous;
    auto & previous = trace[previous_id];

    auto const previous_son_id = previous.son();
    assert(previous_son_id.isValid());
    if (last.son() == previous_son_id) {
        removeNext(trace, previous_id, last_id);
        previous.setLoop(previous.loop() + last.loop());

        detachAndReleaseNode(trace, last_id);

        return previous_id;
    } else {
        return last_id;
    }
}

template <typename Allocator>
auto insertNode(Trace<Allocator> & trace, NodeId next_id, NodeId last_id) -> NodeId {
    assert(next_id.isValid());
    assert(last_id.isValid());
    // auto & next = trace[next_id];
    auto pLast = &trace[last_id];

    auto const last_son_id = pLast->son();
    if (/*last_son_id.isValid() && */ last_son_id == next_id) {  // Implicit if last_son_id is valid
        pLast->setLoop(pLast->loop() + 1);
        return last_id;
    } else {
        auto parent_id = searchParentPattern(trace, last_id, next_id);
        if (!parent_id.isValid()) {
            auto const new_id = trace.newNode();
            setNext(trace, last_id, new_id);
            setSon(trace, new_id, next_id);
            return new_id;
        } else {
            auto pParent = &trace[parent_id];

            // ------------------------
            // Equalize loops if needed

            {
                auto & parent_next = trace[pParent->next];
                auto const parent_next_loop = parent_next.loop();

                if (parent_next_loop > 1u) {
                    auto const new_node_id = trace.newNode();
                    parent_next.setLoop(parent_next_loop - 1u);
                    setSon(trace, new_node_id, parent_next.son());
                    transferNext(trace, parent_id, new_node_id);
                    setNext(trace, parent_id, new_node_id);
                }
            }

            {
                auto const parent_loop_count = pParent->loop();
                auto const last_loop_count = pLast->loop();
                if (parent_loop_count < last_loop_count) {
                    auto const new_node_id = trace.newNode();
                    auto & new_node = trace[new_node_id];
                    new_node.setLoop(last_loop_count);
                    pParent->setLoop(parent_loop_count - last_loop_count);
                    setSon(trace, new_node_id, pParent->son());
                    transferNext(trace, parent_id, new_node_id);
                    setNext(trace, parent_id, new_node_id);
                    parent_id = new_node_id;
                    pParent = &new_node;
                } else if (last_loop_count < parent_loop_count) {
                    auto const new_node_id = trace.newNode();
                    auto & new_node = trace[new_node_id];
                    new_node.setLoop(parent_loop_count);
                    pLast->setLoop(last_loop_count - parent_loop_count);
                    setSon(trace, new_node_id, pLast->son());
                    setNext(trace, last_id, new_node_id);
                    last_id = new_node_id;
                    pLast = &new_node;
                }
            }
            assert(pParent->loop() == pLast->loop());

            // -----------------
            // Factorize pattern

            auto const last_pattern_node_id = getLastPatternNode(trace, pParent->son());
            auto const parent_son_id = pParent->son();
            auto & parent_son = trace[parent_son_id];
            if (!pParent->previous.isValid() && !trace[pParent->next].next.isValid()) {
                if (pParent->loop() == 1u && parent_son.son().isValid() &&
                    parent_son.parents.size() == 1) {
                    // Identify larger known pattern, merge previous
                    // TODO simplify loop construction
                    auto const parent_next_id = pParent->next;
                    removeNext(trace, parent_id, parent_next_id);
                    removeSon(trace, parent_id, parent_son_id);
                    transferValueOrSon(trace, parent_son_id, parent_id);
                    transferNext(trace, parent_son_id, parent_id);
                    pParent->setLoop(parent_son.loop());

                    removeSon(trace, last_id, parent_son_id);
                    trace.releaseNode(parent_son_id);

                    detachAndReleaseNode(trace, parent_next_id);
                    detachAndReleaseNode(trace, last_id);

                    insertNode(trace, sonId(trace, parent_next_id), last_pattern_node_id);
                    return insertNode(trace, parent_id, pLast->previous);
                } else {
                    // Identify larger known pattern, keep previous
                    auto const previous_id = pLast->previous;
                    removeNext(trace, previous_id, last_id);
                    removeSon(trace, last_id, pLast->son());
                    trace.releaseNode(last_id);
                    return insertNode(trace, parent_id, previous_id);
                }
            } else {
                if (!parent_son.isLeaf() && parent_son.parents.size() == 2 && pParent->loop() == 1u) {
                    auto const parent_next_id = pParent->next;
                    auto & last_pattern_node = trace[last_pattern_node_id];

                    if (last_pattern_node.son() == sonId(trace, parent_next_id)) {
                        // Extend node, keeping previous 2
                        removeNext(trace, parent_id, parent_next_id);
                        transferNext(trace, parent_next_id, parent_id);
                        removeSon(trace, parent_next_id, sonId(trace, parent_next_id));
                        trace.releaseNode(parent_next_id);
                        last_pattern_node.setLoop(last_pattern_node.loop() + 1);
                    } else {
                        if (pParent->loop() > 1u) {
                            // Extend node, keeping previous 3
                            auto const new_node_id = trace.newNode();
                            auto & new_node = trace[new_node_id];
                            new_node.setLoop(pParent->loop());
                            pParent->setLoop(1);
                            pLast->setLoop(1);

                            changeSon(trace, parent_id, new_node_id);
                            changeSon(trace, last_id, new_node_id);
                            setSon(trace, new_node_id, pParent->son());

                            transferNext(trace, pParent->next, parent_id, new_node_id);
                        } else {
                            // Extend node, keeping previous 4
                            transferNext(trace, pParent->next, parent_id, last_pattern_node_id);
                        }
                    }
                } else {
                    // Create new pattern
                    auto const new_node_id = trace.newNode();
                    auto & new_node = trace[new_node_id];
                    transferValueOrSon(trace, parent_id, new_node_id);
                    new_node.setLoop(pParent->loop());
                    pParent->setLoop(1);
                    pLast->setLoop(1);
                    transferNext(trace, pParent->next, new_node_id);
                    setSon(trace, parent_id, new_node_id);
                    changeSon(trace, last_id, new_node_id);
                }
                return factorizeLoop(trace, last_id);
            }
        }
    }
}

template <typename Allocator> class TraceBuilder final {
  public:
    auto newLeaf() { return _trace.newLeaf(); }
    auto insert(LeafId leaf_id) -> void {
        if (_last.isValid()) {
            auto const node_id = _trace[leaf_id];
            _last = insertNode(_trace, node_id, _last);
        } else {
            _last = _trace.newNode();
            setSon(_trace, _last, _trace[leaf_id]);
            _trace.setRoot(_last);
        }
    }

    auto trace() const -> Trace<Allocator> const & { return _trace; }

  private:
    Trace<Allocator> _trace;
    NodeId _last = NodeId::invalid();
};

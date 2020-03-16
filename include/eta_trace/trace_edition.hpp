#pragma once

#include "trace.hpp"

template <typename Allocator> auto sonId(Trace<Allocator> const & trace, NodeId node_id) -> NodeId {
    if (node_id.isValid()) {
        auto const & node = trace[node_id];
        return node.son();
    } else {
        return NodeId::invalid();
    }
}

template <typename Allocator>
auto updatePreviousSon(Trace<Allocator> & trace, NodeId previous_id, NodeId old_son_id, NodeId new_son_id)
      -> void {
    if (!previous_id.isValid() || old_son_id == new_son_id)
        return;

    auto & previous_node = trace[previous_id];
    auto const previous_son_id = previous_node.son();
    if (previous_son_id.isValid()) {
        auto & previous_son_node = trace[previous_son_id];
        previous_son_node.parents.replace(previous_id, old_son_id, new_son_id);
    }
}

template <typename Allocator>
auto setNext(Trace<Allocator> & trace, NodeId node_id, NodeId next_id) -> void {
    auto & node = trace[node_id];
    auto & next = trace[next_id];
    assert(!node.next.isValid());
    assert(!next.previous.isValid());

    auto const son_id = node.son();
    if (son_id.isValid()) {
        auto & son = trace[son_id];
        son.parents.replace(node_id, NodeId::invalid(), next.son());
    }
    node.next = next_id;
    next.previous = node_id;
}

template <typename Allocator>
auto setSon(Trace<Allocator> & trace, NodeId node_id, NodeId son_id) -> void {
    auto & node = trace[node_id];
    auto & son = trace[son_id];
    assert(!node.son().isValid());
    node.setSon(son_id);
    updatePreviousSon(trace, node.previous, NodeId::invalid(), son_id);
    son.parents.insert(trace.allocator(), node_id, sonId(trace, node.next));
}

template <typename Allocator>
auto transferValueOrSon(Trace<Allocator> & trace, NodeId from_id, NodeId to_id) -> void {
    auto & from = trace[from_id];
    auto & to = trace[to_id];
    assert(!to.isLeaf() && !to.son().isValid());

    if (from.isLeaf()) {
        assert(!from.previous.isValid());
        assert(!from.next.isValid());
        auto const leaf_id = from.value();
        to.setLeaf(leaf_id);
        from.setSon(NodeId::invalid());
        trace.setLeaf(leaf_id, to_id);
    } else {
        auto const son_id = from.son();
        assert(son_id.isValid());
        updatePreviousSon(trace, from.previous, son_id, NodeId::invalid());
        updatePreviousSon(trace, to.previous, NodeId::invalid(), son_id);
        auto & son = trace[son_id];
        son.parents.remove(from_id, sonId(trace, from.next));
        son.parents.insert(trace.allocator(), to_id, sonId(trace, to.next));
        son.setSon(from.son());
        from.setSon(NodeId::invalid());
    }
}

template <typename Allocator>
auto transferNext(Trace<Allocator> & trace, NodeId from_id, NodeId to_id) -> void {
    auto & from = trace[from_id];
    auto & to = trace[to_id];
    auto & next = trace[from.next];

    if (from.next.isValid()) {
        auto const from_son_id = from.son();
        auto const to_son_id = to.son();

        if (from_son_id.isValid())
            trace[from_son_id].parents.replace(from_id, next.son(), NodeId::invalid());

        if (to_son_id.isValid())
            trace[to_son_id].parents.replace(to_id, NodeId::invalid(), next.son());

        next.previous = to_id;
        to.next = from.next;
        from.next = NodeId::invalid();
    }
}

template <typename Allocator>
auto transferNext(Trace<Allocator> & trace, NodeId n1, NodeId n2, NodeId n3) -> void {
    transferNext(trace, n2, n3);
    transferNext(trace, n1, n2);
}

template <typename Allocator>
auto removeNext(Trace<Allocator> & trace, NodeId from_id, NodeId to_id) -> void {
    auto & from = trace[from_id];
    auto & to = trace[to_id];
    assert(from.next == to_id);
    auto const son_id = from.son();

    if (son_id.isValid())
        trace[son_id].parents.replace(from_id, to_id, NodeId::invalid());

    to.previous = NodeId::invalid();
    from.next = NodeId::invalid();
}

template <typename Allocator>
auto removeSon(Trace<Allocator> & trace, NodeId parent_id, NodeId son_id) -> void {
    auto & parent = trace[parent_id];
    auto & son = trace[son_id];
    assert(parent.son().isValid());
    updatePreviousSon(trace, parent.previous, son_id, NodeId::invalid());
    son.parents.remove(parent_id, sonId(trace, parent.next));
}

template <typename Allocator>
auto changeNext(Trace<Allocator> & trace, NodeId node_id, NodeId new_next_id) -> void {
    auto & node = trace[node_id];
    auto & new_next = trace[new_next_id];
    assert(!new_next.previous.isValid());

    auto const old_next_id = node.next;
    assert(old_next_id.isValid());
    auto & old_next = trace[old_next_id];

    auto const son_id = node.son();
    if (son_id.isValid())
        trace[son_id].parents.replace(node_id, old_next_id, new_next_id);

    node.next = new_next_id;
    new_next.previous = node_id;
}

template <typename Allocator>
auto changeSon(Trace<Allocator> & trace, NodeId parent_id, NodeId new_son_id) -> void {
    auto & parent = trace[parent_id];
    auto const old_son_id = parent.son();
    auto & old_son = trace[old_son_id];
    auto & new_son = trace[new_son_id];

    auto const next_son_id = sonId(trace, parent.next);
    old_son.parents.remove(parent_id, next_son_id);
    new_son.parents.insert(trace.allocator(), parent_id, next_son_id);
    updatePreviousSon(trace, parent.previous, old_son_id, new_son_id);
    parent.setSon(new_son_id);
}

template <typename Allocator>
auto detachAndReleaseNode(Trace<Allocator> & trace, NodeId node_id) -> void {
    assert(node_id.isValid());
    auto & node = trace[node_id];
    if (node.previous.isValid())
        removeNext(trace, node.previous, node_id);
    if (node.next.isValid())
        removeNext(trace, node_id, node.next);
    auto const son_id = node.son();
    if (son_id.isValid())
        removeSon(trace, node_id, son_id);
    trace.releaseNode(node_id);
}

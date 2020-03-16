#include "eta_trace/parents_tree.hpp"

#include <iostream>
#include <vector>

#include "eta_trace/allocator.hpp"

auto show = [](auto const & p) {
    for (auto const & v : p.data()) {
        if (v.key.isValid())
            std::cout << v.key.value() << ':' << v.value.value() << ", ";
        else
            std::cout << "#:" << v.value.value() << ", ";
    }
    std::cout << std::endl;
};

auto compare = [](auto const & a, auto const & b) {
    auto it_a = a.begin();
    auto const end_a = a.end();
    auto it_b = b.begin();
    auto const end_b = b.end();
    while (it_a != end_a && it_b != end_b) {
        if (*it_a != *it_b)
            return false;
        ++it_a;
        ++it_b;
    }
    return !(it_a != end_a) && it_b == end_b;
};

auto main() -> int {
    auto a = StdAllocator {};
    auto p = Parents {};

    p.insert(a, NodeId(4));
    assert(compare(p, std::vector { 4 }));
    p.insert(a, NodeId(9));
    assert(compare(p, std::vector { 4, 9 }));
    p.insert(a, NodeId(6));
    assert(compare(p, std::vector { 4, 6, 9 }));
    p.insert(a, NodeId(3));
    assert(compare(p, std::vector { 3, 4, 6, 9 }));
    p.insert(a, NodeId(15));
    assert(compare(p, std::vector { 3, 4, 6, 9, 15 }));

    p.remove(3);
    assert(compare(p, std::vector { 4, 6, 9, 15 }));
    p.remove(6);
    assert(compare(p, std::vector { 4, 9, 15 }));
    p.remove(15);
    assert(compare(p, std::vector { 4, 9 }));

    assert(!p[NodeId(3)].isValid());
    p.insert(a, NodeId(7), NodeId(3));
    assert(p[NodeId(3)] == NodeId(7));
    assert(compare(p, std::vector { 7, 4, 9 }));

    assert(!p[NodeId(8)].isValid());
    assert(!p[NodeId(2)].isValid());
    p.insert(a, NodeId(5), NodeId(8));
    p.insert(a, NodeId(1), NodeId(2));
    assert(p[NodeId(8)] == NodeId(5));
    assert(p[NodeId(2)] == NodeId(1));
    assert(compare(p, std::vector { 1, 7, 5, 4, 9 }));

    p.insert(a, NodeId(2), NodeId(5));
    p.insert(a, NodeId(8), NodeId(6));
    assert(compare(p, std::vector { 1, 7, 2, 8, 5, 4, 9 }));

    assert(p[NodeId(3)] == NodeId(7));
    p.remove(NodeId(7), NodeId(3));
    assert(compare(p, std::vector { 1, 2, 8, 5, 4, 9 }));
    assert(!p[NodeId(3)].isValid());

    assert(p[NodeId(6)] == NodeId(8));
    p.remove(NodeId(8), NodeId(6));
    assert(compare(p, std::vector { 1, 2, 5, 4, 9 }));
    assert(!p[NodeId(6)].isValid());

    assert(p[NodeId(2)] == NodeId(1));
    p.remove(NodeId(1), NodeId(2));
    assert(compare(p, std::vector { 2, 5, 4, 9 }));
    assert(!p[NodeId(2)].isValid());

    p.deinit(a);

    p.insert(a, NodeId(3));
    p.insert(a, NodeId(9));
    p.insert(a, NodeId(5));
    p.insert(a, NodeId(2), NodeId(6));
    p.insert(a, NodeId(8), NodeId(14));
    p.insert(a, NodeId(6), NodeId(10));
    p.insert(a, NodeId(4), NodeId(8));
    assert(compare(p, std::vector { 2, 4, 6, 8, 3, 5, 9 }));

    assert(p[NodeId(14)] == NodeId(8));
    assert(p[NodeId(0)] == NodeId::invalid());
    assert(p[NodeId(8)] == NodeId(4));
    assert(p[NodeId(1)] == NodeId::invalid());
    p.replace(NodeId(8), NodeId(14), NodeId(0));
    p.replace(NodeId(4), NodeId(8), NodeId(1));
    assert(compare(p, std::vector { 8, 4, 2, 6, 3, 5, 9 }));
    assert(p[NodeId(14)] == NodeId::invalid());
    assert(p[NodeId(0)] == NodeId(8));
    assert(p[NodeId(8)] == NodeId::invalid());
    assert(p[NodeId(1)] == NodeId(4));

    p.deinit(a);

    return 0;
}


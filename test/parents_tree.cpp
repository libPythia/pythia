#include "eta/trace/parents_tree.hpp"

#include <iostream>
#include <vector>

#include "eta/core/allocator.hpp"
#include "eta/trace/trace.hpp"

auto show = [](auto const & p) {
    for (auto const & v : p.data()) {
        if (v.key != nullptr)
            std::cout << reinterpret_cast<std::size_t>(v.key) << ':'
                      << reinterpret_cast<std::size_t>(v.value) << ", ";
        else
            std::cout << "∅:" << reinterpret_cast<std::size_t>(v.value) << ", ";
    }
    std::cout << " (" << p.keyCount() << " keys)" << std::endl;
};

auto compare = [](auto const & a, auto const & b) {
    auto it_a = a.begin();
    auto const end_a = a.end();
    auto it_b = b.begin();
    auto const end_b = b.end();
    while (it_a != end_a && it_b != end_b) {
        if (*it_a != reinterpret_cast<Node *>(*it_b))
            return false;
        ++it_a;
        ++it_b;
    }
    return !(it_a != end_a) && it_b == end_b;
};

auto main() -> int {
    auto a = StdAllocator {};
    auto p = Parents {};

    p.insert(a, reinterpret_cast<Node *>(4));
    assert(compare(p, std::vector { 4 }));
    assert(p.keyCount() == 0);
    p.insert(a, reinterpret_cast<Node *>(9));
    assert(compare(p, std::vector { 4, 9 }));
    p.insert(a, reinterpret_cast<Node *>(6));
    assert(compare(p, std::vector { 4, 6, 9 }));
    p.insert(a, reinterpret_cast<Node *>(3));
    assert(compare(p, std::vector { 3, 4, 6, 9 }));
    p.insert(a, reinterpret_cast<Node *>(15));
    assert(compare(p, std::vector { 3, 4, 6, 9, 15 }));
    assert(p.keyCount() == 0);

    p.remove(reinterpret_cast<Node *>(3));
    assert(compare(p, std::vector { 4, 6, 9, 15 }));
    p.remove(reinterpret_cast<Node *>(6));
    assert(compare(p, std::vector { 4, 9, 15 }));
    p.remove(reinterpret_cast<Node *>(15));
    assert(compare(p, std::vector { 4, 9 }));
    assert(p.keyCount() == 0);

    assert(p[reinterpret_cast<Node *>(3)] == nullptr);
    p.insert(a, reinterpret_cast<Node *>(7), reinterpret_cast<Node *>(3));
    assert(p[reinterpret_cast<Node *>(3)] == reinterpret_cast<Node *>(7));
    assert(compare(p, std::vector { 7, 4, 9 }));
    assert(p.keyCount() == 1);

    assert(p[reinterpret_cast<Node *>(8)] == nullptr);
    assert(p[reinterpret_cast<Node *>(2)] == nullptr);
    p.insert(a, reinterpret_cast<Node *>(5), reinterpret_cast<Node *>(8));
    p.insert(a, reinterpret_cast<Node *>(1), reinterpret_cast<Node *>(2));
    assert(p.keyCount() == 3);
    assert(p[reinterpret_cast<Node *>(8)] == reinterpret_cast<Node *>(5));
    assert(p[reinterpret_cast<Node *>(2)] == reinterpret_cast<Node *>(1));
    assert(compare(p, std::vector { 1, 7, 5, 4, 9 }));

    p.insert(a, reinterpret_cast<Node *>(2), reinterpret_cast<Node *>(5));
    p.insert(a, reinterpret_cast<Node *>(8), reinterpret_cast<Node *>(6));
    assert(compare(p, std::vector { 1, 7, 2, 8, 5, 4, 9 }));
    assert(p.keyCount() == 5);

    assert(p[reinterpret_cast<Node *>(3)] == reinterpret_cast<Node *>(7));
    p.remove(reinterpret_cast<Node *>(7), reinterpret_cast<Node *>(3));
    assert(compare(p, std::vector { 1, 2, 8, 5, 4, 9 }));
    assert(p[reinterpret_cast<Node *>(3)] == nullptr);
    assert(p.keyCount() == 4);

    assert(p[reinterpret_cast<Node *>(6)] == reinterpret_cast<Node *>(8));
    p.remove(reinterpret_cast<Node *>(8), reinterpret_cast<Node *>(6));
    assert(compare(p, std::vector { 1, 2, 5, 4, 9 }));
    assert(p[reinterpret_cast<Node *>(6)] == nullptr);
    assert(p.keyCount() == 3);

    assert(p[reinterpret_cast<Node *>(2)] == reinterpret_cast<Node *>(1));
    p.remove(reinterpret_cast<Node *>(1), reinterpret_cast<Node *>(2));
    assert(compare(p, std::vector { 2, 5, 4, 9 }));
    assert(p[reinterpret_cast<Node *>(2)] == nullptr);
    assert(p.keyCount() == 2);

    p.deinit(a);

    p.insert(a, reinterpret_cast<Node *>(3));
    p.insert(a, reinterpret_cast<Node *>(9));
    p.insert(a, reinterpret_cast<Node *>(5));
    p.insert(a, reinterpret_cast<Node *>(2), reinterpret_cast<Node *>(6));
    p.insert(a, reinterpret_cast<Node *>(8), reinterpret_cast<Node *>(14));
    p.insert(a, reinterpret_cast<Node *>(6), reinterpret_cast<Node *>(10));
    p.insert(a, reinterpret_cast<Node *>(4), reinterpret_cast<Node *>(8));

    assert(p[reinterpret_cast<Node *>(14)] == reinterpret_cast<Node *>(8));
    assert(p[reinterpret_cast<Node *>(1)] == nullptr);
    assert(p[reinterpret_cast<Node *>(8)] == reinterpret_cast<Node *>(4));
    assert(p[reinterpret_cast<Node *>(2)] == nullptr);
    assert(compare(p, std::vector { 2, 4, 6, 8, 3, 5, 9 }));
    assert(p.keyCount() == 4);

    p.replace(reinterpret_cast<Node *>(8), reinterpret_cast<Node *>(14), reinterpret_cast<Node *>(1));
    p.replace(reinterpret_cast<Node *>(4), reinterpret_cast<Node *>(8), reinterpret_cast<Node *>(2));

    assert(p[reinterpret_cast<Node *>(14)] == nullptr);
    assert(p[reinterpret_cast<Node *>(1)] == reinterpret_cast<Node *>(8));
    assert(p[reinterpret_cast<Node *>(8)] == nullptr);
    assert(p[reinterpret_cast<Node *>(2)] == reinterpret_cast<Node *>(4));
    assert(compare(p, std::vector { 8, 4, 2, 6, 3, 5, 9 }));
    assert(p.keyCount() == 4);

    show(p);
    p.replace(reinterpret_cast<Node *>(8), reinterpret_cast<Node *>(1), nullptr);
    show(p);
    assert(p.keyCount() == 3);
    p.replace(reinterpret_cast<Node *>(9), nullptr, reinterpret_cast<Node *>(1));
    show(p);
    assert(p.keyCount() == 4);

    p.deinit(a);

    return 0;
}


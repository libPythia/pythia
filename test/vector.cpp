#include "eta_trace/impl/vector.hpp"

#include <cassert>
#include <iostream>
#include <vector>

#include "eta_trace/allocator.hpp"

auto is_equal = [](auto const & a, auto const & b) {
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
    return it_a == end_a && it_b == end_b;
};

auto show = [](auto const & v) {
    for (auto i : v)
        std::cout << i << ", ";
    std::cout << std::endl;
};

auto main() -> int {
    auto a = StdAllocator {};
    auto v = vector<int> {};
    assert(v.capacity() == 0);
    assert(v.size() == 0);

    {
        auto count = 0;
        for (auto const & i : v)
            count += i;
        assert(count == 0);
    }

    v.push_back(a, 2);
    assert(v.size() == 1);
    assert(v.capacity() == 4);
    assert(is_equal(v, std::vector { 2 }));

    v.push_back(a, 4);
    assert(v.size() == 2);
    assert(v.capacity() == 4);
    assert(is_equal(v, std::vector { 2, 4 }));

    v.push_back(a, 6);
    assert(v.size() == 3);
    assert(v.capacity() == 4);
    assert(is_equal(v, std::vector { 2, 4, 6 }));

    v.push_back(a, 8);
    assert(v.size() == 4);
    assert(v.capacity() == 4);
    assert(is_equal(v, std::vector { 2, 4, 6, 8 }));

    {
        auto count = 0;
        auto const s = static_cast<int>(v.size());
        for (auto i = 0; i < s; ++i) {
            assert(v[i] == 2 * (i + 1));
            count += v[i];
        }
        assert(count == 20);
    }

    v.push_back(a, 10);
    assert(v.size() == 5);
    assert(v.capacity() == 8);
    assert(is_equal(v, std::vector { 2, 4, 6, 8, 10 }));

    v.push_back(a, 12);
    assert(v.size() == 6);
    assert(v.capacity() == 8);
    assert(is_equal(v, std::vector { 2, 4, 6, 8, 10, 12 }));

    {
        auto count = 0;
        auto const s = static_cast<int>(v.size());
        for (auto i = 0; i < s; ++i) {
            assert(v[i] == 2 * (i + 1));
            count += v[i];
        }
        assert(count == 42);
    }

    {
        auto const s = v.size();
        v.shrink(a);
        assert(v.size() == s);
        assert(v.size() == v.capacity());
        assert(is_equal(v, std::vector { 2, 4, 6, 8, 10, 12 }));
    }

    v.pop_back();
    assert(v.size() == 5);
    assert(v.capacity() == 6);
    assert(is_equal(v, std::vector { 2, 4, 6, 8, 10 }));

    v.insert(a, 3, 5);
    assert(v.size() == 6);
    assert(v.capacity() == 6);
    assert(is_equal(v, std::vector { 2, 4, 6, 5, 8, 10 }));

    v.insert(a, 0, 3);
    assert(v.size() == 7);
    assert(v.capacity() == 12);
    assert(is_equal(v, std::vector { 3, 2, 4, 6, 5, 8, 10 }));

    v.insert(a, v.size(), 7);
    assert(v.size() == 8);
    assert(v.capacity() == 12);
    assert(is_equal(v, std::vector { 3, 2, 4, 6, 5, 8, 10, 7 }));

    v.remove(4);
    assert(v.size() == 7);
    assert(v.capacity() == 12);
    assert(is_equal(v, std::vector { 3, 2, 4, 6, 8, 10, 7 }));

    v.remove(0);
    assert(v.size() == 6);
    assert(v.capacity() == 12);
    assert(is_equal(v, std::vector { 2, 4, 6, 8, 10, 7 }));

    v.remove(v.size() - 1);
    assert(v.size() == 5);
    assert(v.capacity() == 12);
    assert(is_equal(v, std::vector { 2, 4, 6, 8, 10 }));

    v.deinit(a);
}


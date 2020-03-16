#pragma once

#include <cstring>

#include "ids.hpp"
#include "vector.hpp"

namespace ParentsImplementation {
inline auto find_index = [](auto id, auto m, auto M, auto && key) {
    while (m < M) {
        auto const pivot = (m + M) / 2;
        auto const k = key(pivot);
        if (k < id) {
            m = pivot + 1;
        } else if (id < k) {
            M = pivot;
        } else {
            return pivot;
        }
    }
    assert(m == M);
    return M;
};
}

struct Parents {
    using index_t = NodeId::underlying_t;

    struct pair {
        NodeId key = NodeId::invalid();
        NodeId value = NodeId::invalid();
    };

  public:
    class const_iterator final {
      private:
        friend class Parents;
        const_iterator(pair const * it)
              : _it(it) {}

      public:
        auto operator*() { return _it->value; }
        auto operator-> () { return &_it->value; }
        auto operator++() { return ++_it; }
        auto operator!=(const_iterator const & it) { return _it != it._it; }

      private:
        pair const * _it;
    };

  public:
    auto begin() const { return const_iterator(_data.begin()); }
    auto end() const { return const_iterator(_data.end()); }

    auto data() const -> vector<pair> const & { return _data; }

    auto operator[](NodeId id) const -> NodeId {
        auto const index = ParentsImplementation::find_index(id, 0u, _key_count, [&](auto i) {
            return _data[i].key;
        });
        if (index == _key_count || _data[index].key != id)
            return NodeId::invalid();
        return _data[index].value;
    }

    template <typename Allocator>
    auto insert(Allocator & alloc, NodeId id, NodeId key = NodeId::invalid()) -> void {
        if (key.isValid()) {
            auto const index = ParentsImplementation::find_index(key, 0u, _key_count, [&](auto i) {
                return _data[i].key;
            });
            assert(_data[index].key != key);
            _data.insert(alloc, index, pair { key, id });
            ++_key_count;
        } else {
            auto const index =
                  ParentsImplementation::find_index(id, _key_count, _data.size(), [&](auto i) {
                      return _data[i].value;
                  });
            assert(index >= _key_count && (index == _data.size() || _data[index].value != id));
            _data.insert(alloc, index, pair { NodeId::invalid(), id });
        }
    }

    auto remove(NodeId value, NodeId key = NodeId::invalid()) -> void {
        if (key.isValid()) {
            auto const index = ParentsImplementation::find_index(key, 0u, _key_count, [&](auto i) {
                return _data[i].key;
            });
            assert(_data[index].key == key);
            assert(_data[index].value == value);
            _data.remove(index);
            --_key_count;
        } else {
            auto const index =
                  ParentsImplementation::find_index(value, _key_count, _data.size(), [&](auto i) {
                      return _data[i].value;
                  });
            assert(_data[index].value == value);
            _data.remove(index);
        }
    }

    auto replace(NodeId value, NodeId old_key, NodeId new_key) -> void {
        if (old_key == new_key)
            return;

        auto const old_index = [&]() {
            if (old_key.isValid())
                return ParentsImplementation::find_index(old_key, 0u, _key_count, [&](auto i) {
                    return _data[i].key;
                });
            else
                return ParentsImplementation::find_index(value, _key_count, _data.size(), [&](auto i) {
                    return _data[i].value;
                });
        }();

        auto const new_index = [&]() {
            if (new_key.isValid())
                return ParentsImplementation::find_index(new_key, 0u, _key_count, [&](auto i) {
                    return _data[i].key;
                });
            else
                return ParentsImplementation::find_index(value, _key_count, _data.size(), [&](auto i) {
                    return _data[i].value;
                });
        }();

        assert(_data[old_index].key == old_key);
        assert(_data[old_index].value == value);

        auto const new_it = _data.begin() + new_index;

        if (old_index < new_index) {
            auto const old_it = _data.begin() + old_index;
            std::memmove(old_it, old_it + 1, (new_index - old_index) * sizeof(pair));
        } else {
            std::memmove(new_it + 1, new_it, (old_index - new_index) * sizeof(pair));
        }

        new_it->value = value;
        new_it->key = new_key;
    }

    template <typename Allocator> auto deinit(Allocator & alloc) {
        _data.deinit(alloc);
        _key_count = 0u;
    }

    auto size() const { return _data.size(); }

  private:
    vector<pair> _data;
    std::size_t _key_count = 0u;
};

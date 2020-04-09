#pragma once

#include "eta/core/allocator.hpp"
#include "eta/core/vector.hpp"

class Node;

class Parents {
    struct pair {
        Node * key = nullptr;
        Node * value = nullptr;
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
    auto keyCount() const { return _key_count; }

    auto operator[](Node * id) const -> Node *;
    auto insert(GenericAllocator & alloc, Node * node, Node * key = nullptr) -> void;
    auto remove(Node * node, Node * key = nullptr) -> void;
    auto replace(Node * node, Node * old_key, Node * new_key) -> void;

    auto deinit(GenericAllocator & alloc) -> void;

    auto size() const { return _data.size(); }

  private:
    vector<pair> _data;
    std::size_t _key_count = 0u;
};

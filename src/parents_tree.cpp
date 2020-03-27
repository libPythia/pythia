#include "eta_trace/impl/parents_tree.hpp"

#undef assert
#define assert(cond) \
    if (!(cond))     \
        throw 1;     \
    else {           \
    }
// ------------------------------------------------

template <typename F> static auto find_index(Node * id, std::size_t m, std::size_t M, F && key) {
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
}

// ------------------------------------------------

auto Parents::operator[](Node * id) const -> Node * {
    auto const index = find_index(id, 0u, _key_count, [&](auto i) { return _data[i].key; });
    if (index == _key_count || _data[index].key != id)
        return nullptr;
    return _data[index].value;
}

// ------------------------------------------------

auto Parents::insert(GenericAllocator & alloc, Node * node, Node * key) -> void {
    assert(node != nullptr);
    if (key != nullptr) {
        auto const index = find_index(key, 0u, _key_count, [&](auto i) { return _data[i].key; });
        assert(index == _data.size() || _data[index].key != key);
        _data.insert(alloc, index, pair { key, node });
        ++_key_count;
    } else {
        auto const index =
              find_index(node, _key_count, _data.size(), [&](auto i) { return _data[i].value; });
        assert(index >= _key_count && (index == _data.size() || _data[index].value != node));
        _data.insert(alloc, index, pair { nullptr, node });
    }
}

// ------------------------------------------------

auto Parents::remove(Node * node, Node * key) -> void {
    assert(node != nullptr);
    if (key != nullptr) {
        auto const index = find_index(key, 0u, _key_count, [&](auto i) { return _data[i].key; });
        assert(_data[index].key == key);
        assert(_data[index].value == node);
        _data.remove(index);
        --_key_count;
    } else {
        auto const index =
              find_index(node, _key_count, _data.size(), [&](auto i) { return _data[i].value; });
        assert(_data[index].value == node);
        _data.remove(index);
    }
}

// ------------------------------------------------

auto Parents::replace(Node * node, Node * old_key, Node * new_key) -> void {
    assert(node != nullptr);
    if (old_key == new_key)
        return;

    auto const old_index = [&]() {
        if (old_key != nullptr)
            return find_index(old_key, 0u, _key_count, [&](auto i) { return _data[i].key; });
        else
            return find_index(node, _key_count, _data.size(), [&](auto i) { return _data[i].value; });
    }();

    auto const new_index = [&]() {
        if (new_key != nullptr)
            return find_index(new_key, 0u, _key_count, [&](auto i) { return _data[i].key; });
        else
            return find_index(node, _key_count, _data.size(), [&](auto i) { return _data[i].value; });
    }();

    assert(old_index < _data.size());
    assert(_data[old_index].key == old_key);
    assert(_data[old_index].value == node);

    if (old_index < new_index) {
        assert(new_index <= _data.size());
        auto const old_it = _data.begin() + old_index;
        std::memmove(old_it, old_it + 1, (new_index - old_index) * sizeof(pair));

        auto const new_it = _data.begin() + new_index - 1;
        new_it->value = node;
        new_it->key = new_key;
    } else {
        auto const new_it = _data.begin() + new_index;
        std::memmove(new_it + 1, new_it, (old_index - new_index) * sizeof(pair));
        new_it->value = node;
        new_it->key = new_key;
    }

    if (old_key == nullptr) {
        assert(new_key != nullptr);
        ++_key_count;
    } else if (new_key == nullptr)
        --_key_count;
}

// ------------------------------------------------

auto Parents::deinit(GenericAllocator & alloc) -> void {
    _data.deinit(alloc);
    _key_count = 0u;
}

// ------------------------------------------------


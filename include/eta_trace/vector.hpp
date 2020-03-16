#pragma once

#include <cassert>
#include <cstring>
#include <new>
#include <utility>

template <typename T> struct vector final {
  public:
    vector() = default;
    vector(vector && v)
          : _data(v._data) {
        v._data = nullptr;
    }
    vector(vector const &) = delete;
    auto operator=(vector &&) -> vector & = delete;
    auto operator=(vector const &) -> vector & = delete;
    ~vector() { assert(_data == nullptr); }

  public:
    template <typename Alloc, typename... Args>
    auto push_back(Alloc & alloc, Args &&... args) -> void {
        auto const s = size();
        reserve(alloc, s + 1);
        new (begin() + s) T(std::forward<Args>(args)...);
        set_size(s + 1);
    }

    template <typename Alloc, typename... Args>
    auto insert(Alloc & alloc, std::size_t index, Args &&... args) -> void {
        auto const s = size();
        reserve(alloc, s + 1);
        auto const it = begin() + index;
        std::memmove(it + 1, it, (s - index) * sizeof(T));
        new (it) T(std::forward<Args>(args)...);
        set_size(s + 1);
    }

    auto pop_back() -> void {
        auto const s = size() - 1;
        begin()[s].~T();
        set_size(s);
    }

    auto remove(std::size_t index) -> void {
        auto const it = begin() + index;
        auto const s = size() - 1;
        it->~T();
        std::memmove(it, it + 1, (s - index) * sizeof(T));
        set_size(s);
    }

    auto clear() -> void {
        if (size() > 0u) {
            auto const _end = end();
            for (auto it = begin(); it != _end; ++it)
                it->~T();
            set_size(0u);
        }
    }

    template <typename Alloc> auto deinit(Alloc & alloc) -> void {
        clear();
        shrink(alloc);
    }

    template <typename Alloc> auto shrink(Alloc & alloc) {
        auto const s = size();
        if (s == 0u) {
            alloc.dealloc(_data);
            _data = nullptr;
        } else {
            _data = alloc.realloc(_data, 2 * sizeof(std::size_t) + s * sizeof(T));
            set_capacity(s);
        }
    }

    template <typename Alloc> auto reserve_exactly(Alloc & alloc, std::size_t size) {
        if (size > capacity()) {
            _data = alloc.realloc(_data, 2 * sizeof(std::size_t) + size * sizeof(T));
            assert(_data != nullptr);
            set_capacity(size);
        }
    }

    template <typename Alloc> auto reserve(Alloc & alloc, std::size_t size) {
        auto max = [](auto a, auto b) { return a < b ? b : a; };
        auto c = max(capacity(), 4u);
        while (c < size)
            c *= 2;
        reserve_exactly(alloc, c);
    }

    auto size() const -> std::size_t {
        if (_data == nullptr)
            return 0u;
        return reinterpret_cast<std::size_t const *>(_data)[0];
    }

    auto capacity() const -> std::size_t {
        if (_data == nullptr)
            return 0u;
        return reinterpret_cast<std::size_t const *>(_data)[1];
    }

    auto begin() -> T * { return (T *)(((std::size_t *)_data) + 2u); }
    auto end() -> T * { return begin() + size(); }
    auto begin() const -> T const * { return (T const *)(((std::size_t const *)_data) + 2u); }
    auto end() const -> T const * { return begin() + size(); }

    auto back() -> T & { return operator[](size() - 1); }
    auto back() const -> T const & { return operator[](size() - 1); }

    auto operator[](std::size_t i) -> T & {
        assert(i < size());
        return begin()[i];
    }
    auto operator[](std::size_t i) const -> T const & {
        assert(i < size());
        return begin()[i];
    }

  private:
    auto set_size(std::size_t s) {
        assert(_data != nullptr);
        static_cast<std::size_t *>(_data)[0] = s;
    }
    auto set_capacity(std::size_t s) {
        assert(_data != nullptr);
        static_cast<std::size_t *>(_data)[1] = s;
    }

  private:
    void * _data = nullptr;
};

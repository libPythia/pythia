#pragma once

#include <cassert>
#include <cstring>
#include <new>
#include <utility>

#include "vector.hpp"

template <typename T, std::size_t chunk_size> struct chunk_vector final {
  public:
    ~chunk_vector() { assert(_data.capacity() == 0u); }

  public:
    template <typename Alloc, typename... Args> auto push_back(Alloc & alloc, Args &&... args) {
        reserve(alloc, _size + 1);
        auto const s = _size++;
        new (&operator[](s)) T(std::forward<Args>(args)...);
    }

    auto pop_back() {
        assert(_size > 0);
        operator[](_size - 1).~T();
        --_size;
    }

    auto clear() {
        if (size() > 0u) {
            auto const last_chunk_size = _size % chunk_size;
            auto const full_chunk_count = _size / chunk_size;

            auto destroy_chunk = [&](auto it, auto c_size) {
                auto const end = it + c_size;
                for (; it != end; ++it)
                    it->~T();
            };

            for (auto i = 0u; i < full_chunk_count; ++i)
                destroy_chunk(_data[i], chunk_size);

            if (last_chunk_size != 0)
                destroy_chunk(_data[full_chunk_count], last_chunk_size);

            _size = 0u;
        }
    }

    template <typename Alloc> auto deinit(Alloc & alloc) -> void {
        clear();
        shrink(alloc);
    }

    template <typename Alloc> auto reserve(Alloc & alloc, std::size_t size) -> void {
        auto const last_chunk_size = size % chunk_size;
        auto const chunk_count = (last_chunk_size == 0 ? size / chunk_size : size / chunk_size + 1);
        while (_data.size() < chunk_count)
            _data.push_back(alloc, static_cast<T *>(alloc.alloc(chunk_size * sizeof(T))));
    }

    auto size() const -> std::size_t { return _size; }
    auto capacity() const -> std::size_t { return _data.size() * chunk_size; }

    template <typename Alloc> auto shrink(Alloc & alloc) {
        if (_size == 0u) {
            for (auto const ptr : _data)
                alloc.dealloc(ptr);
            _data.clear();
        } else {
            auto const chunk_count = [&]() {
                auto const last_chunk_size = _size % chunk_size;
                if (last_chunk_size == 0)
                    return _size / chunk_size;
                return _size / chunk_size + 1;
            }();

            while (_data.size() > chunk_count) {
                alloc.dealloc(_data[_data.size() - 1]);
                _data.pop_back();
            }
        }
        _data.shrink(alloc);
    }

    auto operator[](std::size_t i) -> T & {
        assert(i < _size);
        return _data[i / chunk_size][i % chunk_size];
    }
    auto operator[](std::size_t i) const -> T const & {
        assert(i < _size);
        return _data[i / chunk_size][i % chunk_size];
    }

  private:
    vector<T *> _data;
    std::size_t _size;
};

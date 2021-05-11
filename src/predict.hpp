#pragma once

#include <cassert>
#include <eta/factorization/reduction.hpp>
#include <vector>

struct Path final {
    std::vector < validated
};
using Path = std::vector<GrammarBaseObject const *>;

auto first_terminal(Symbol const * const n) -> Terminal const * {
    if (is_terminal(n)) {
        return static_cast<Terminal const *>(n);
    } else {
        assert(is_nonterminal(n));
        auto const nt = static_cast<NonTerminal const *>(n);
        return first_terminal(nt->first->maps_to);
    }
}

auto init_path(Terminal const * node) -> Path {
    auto res = Path {};
    res.push_back(node);
    return res;
}

auto improve_path(Path p, Terminal const * node) -> Path {
    // TODO
}

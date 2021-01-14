#include <cassert>
#include <eta/factorization/export.hpp>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "colors.hpp"

struct PathNode {
    Node const * node;
    size_t count;
};

using Path = std::vector<PathNode>;

static auto descend_path(Path & p) -> void {
    while (true) {
        auto const last = p.back().node->maps_to;
        if (is_terminal(last))
            return;
        p.emplace_back(PathNode { as_nonterminal(last)->first, 0 });
    }
}

static auto initial_path(Grammar const & g) -> Path {
    auto res = Path { { g.root->first, 0 } };
    descend_path(res);
    return res;
}

static auto next(Path & p) -> void {
    if (++p.back().count == p.back().node->repeats) {
        if (is_node(p.back().node->next)) {
            p.back().count = 0;
            p.back().node = as_node(p.back().node->next);
        } else if (p.size() > 1) {
            p.pop_back();
            next(p);
        }
    }
    descend_path(p);
}

static auto get_terminal(Path const & p) -> Terminal const * {
    auto const last = p.back().node->maps_to;
    return as_terminal(last);
}

auto replay(Grammar const & g, terminal_printer const & printer) -> void {
    std::cout << "    ";
    auto const trace = linearise_grammar(g);

    auto path = initial_path(g);
    auto first = true;
    for (auto const terminal : trace) {
        if (first)
            first = false;
        else
            next(path);

        auto const predicted_terminal = get_terminal(path);
        if (terminal == predicted_terminal)
            set_color(std::cout, color_t::blue);
        else
            set_color(std::cout, color_t::red);

        printer(predicted_terminal, std::cout);
    }

    set_color(std::cout, color_t::standard);
}


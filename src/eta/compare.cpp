#include <cassert>
#include <eta/factorization/export.hpp>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "colors.hpp"

// -----------------------------------------------------------

struct PathNode {
    Node const * node = nullptr;
    size_t count = 0;
    std::unique_ptr<PathNode> parent;

    PathNode(Node const * n, std::unique_ptr<PathNode> p = nullptr);
    ~PathNode();
};

PathNode::PathNode(Node const * n, std::unique_ptr<PathNode> p)
        : node(n)
        , count(0)
        , parent(std::move(p)) {
    assert(node != nullptr);
}
PathNode::~PathNode() = default;

// -----------------------------------------------------------

using Path = std::unique_ptr<PathNode>;

static auto check_possible_paths(std::vector<Path> const & pp, Symbol const * s) -> bool;

static auto get_symbol(Node const * n) -> Symbol const * {
    while (true) {
        if (is_symbol(n->next))
            return as_symbol(n->next);
        n = as_node(n->next);
    }
}

template <typename... Ts> decltype(auto) make_path(Ts &&... args) {
    return std::make_unique<PathNode>(std::forward<Ts>(args)...);
}

static auto check_path(Path const & p, Symbol const * s) -> bool {
    if (p == nullptr) {
        std::cerr << "Empty path." << std::endl;
    } else if (p->node == nullptr) {
        std::cerr << "Missing node." << std::endl;
    } else if (p->count > p->node->repeats) {
        std::cerr << "Inconsistent repetition count." << std::endl;
    } else if (s == nullptr || p->node->maps_to != s) {
        std::cerr << "Unexpected maped symbol in path." << std::endl;
    } else if (p->parent != nullptr && !check_path(p->parent, get_symbol(p->node))) {
        std::cerr << "Wrong parent." << std::endl;
    } else {
        return true;
    }
    return false;
}

static auto get_terminal(Path const & p) -> Terminal const * {
    auto const last = p->node->maps_to;
    return as_terminal(last);
}

// -----------------------------------------------------------

using PossiblePaths = std::vector<Path>;

static auto check_possible_paths(PossiblePaths const & pp, Symbol const * s) -> bool {
    auto const count = pp.size();
    if (count == 0) {
        std::cerr << "No possible path" << std::endl;
        return false;
    }

    for (auto i = 0u; i < count; ++i) {
        if (!check_path(pp[i], s))
            return false;
    }

    return true;
}

// -----------------------------------------------------------

static auto print_symbol(Symbol const * s, terminal_printer const & printer) -> void {
    if (is_terminal(s)) {
        printer(as_terminal(s), std::cerr);
    } else {
        auto const nonterminal = as_nonterminal(s);
        auto n = nonterminal->first;
        while (true) {
            print_symbol(n->maps_to, printer);
            if (!is_node(n->next))
                break;
            n = as_node(n->next);
        }
    }
}

static auto print_path(Path const & p, terminal_printer const & printer) -> int {
    auto indent_count = (p->parent == nullptr) ? 0 : (1 + print_path(p, printer));

    for (auto i = 0; i < indent_count; ++i)
        std::cerr << "    ";
    print_symbol(p->node->maps_to, printer);
    std::cerr << std::endl;
    return indent_count;
}

[[maybe_unused]] static auto print_possible_paths(PossiblePaths const & pp,
                                                  terminal_printer const & printer) {
    std::cerr << "PossiblePaths :" << std::endl;
    for (auto const & p : pp) {
        if (p == nullptr)
            std::cerr << "NULL\n";
        else
            print_path(p, printer);
        std::cerr << "---------------------" << std::endl;
    }
}

// -----------------------------------------------------------

static auto descend_path(Path & p) -> void {
    assert(p->node != nullptr);
    while (true) {
        assert(check_path(p, p->node->maps_to));
        auto const last = p->node->maps_to;
        if (is_terminal(last))
            return;
        p = make_path(as_nonterminal(last)->first, std::move(std::move(p)));
    }
}

static auto initial_path(Grammar const & g) -> Path {
    assert(g.root != nullptr);
    auto res = make_path(g.root->first);
    descend_path(res);
    assert(check_path(res, res->node->maps_to));
    assert(is_terminal(res->node->maps_to));
    [[maybe_unused]] auto test = [&g](auto rec, auto const & n) -> bool {
        return (n->node == g.root->first) || (n->parent != nullptr && rec(rec, n->parent));
    };
    assert(test(test, res));
    return res;
}

// -----------------------------------------------------------

enum class result_t {
    success,
    luck,
    failed,
};

static auto possibles_paths_from_terminal(PossiblePaths & pp, Terminal const * t) -> void {
    for (auto const p : t->occurences_with_successor)
        pp.emplace_back(make_path(p.second));
    for (auto const p : t->occurences_without_successor)
        pp.emplace_back(make_path(p));
}

static auto next_path(PossiblePaths & pp, Path p, Terminal const * t, result_t & r) -> void {
    assert(p != nullptr);
    assert(p->node != nullptr);
    ++p->count;

    if (p->count < p->node->repeats) {
    } else if (is_node(p->node->next)) {
        p->node = as_node(p->node->next);
        p->count = 0;
    } else if (p->parent != nullptr) {
        next_path(pp, std::move(p->parent), t, r);
        return;
    } else {
        auto const s = get_symbol(p->node);
        for (auto const n : s->occurences_without_successor)
            next_path(pp, make_path(n), t, r);
        for (auto const n : s->occurences_with_successor)
            next_path(pp, make_path(n.second), t, r);
        return;
    }
    descend_path(p);
    if (get_terminal(p) == t)
        pp.emplace_back(std::move(p));
    else
        r = result_t::luck;
}

static auto next(Grammar const & g, PossiblePaths & possible_paths, Terminal const * t)
        -> result_t {
    auto result = result_t::success;
    auto res = std::vector<Path> {};

    // Special case for initialization on first event
    if (possible_paths.empty()) {
        auto p = initial_path(g);
        if (get_terminal(p) == t) {
            res.emplace_back(std::move(p));
            result = result_t::success;
        } else {
            possibles_paths_from_terminal(res, t);
            result = result_t::failed;
        }
    } else
        for (auto & pp : possible_paths)
            next_path(res, std::move(pp), t, result);

    if (res.empty()) {
        possibles_paths_from_terminal(res, t);
        result = result_t::failed;
    }
    assert(check_possible_paths(res, t));
    possible_paths = std::move(res);
    return result;
}

// -----------------------------------------------------------

auto compare(Grammar const & g, std::string const & str, terminal_printer const & printer) -> void {
    std::cout << std::endl;

    auto const trace = [&]() {
        auto index = std::unordered_map<char, Terminal const *> {};
        for (auto const & terminal : g.terminals) {
            auto ss = std::stringstream {};
            printer(terminal.get(), ss);
            assert(ss.str().size() == 1);
            index.emplace(ss.str()[0], terminal.get());
        }

        auto tmp = std::vector<std::pair<Terminal const *, char>> {};
        for (auto const c : str) {
            auto const it = index.find(c);
            if (it != index.end())
                tmp.emplace_back(it->second, c);
            else
                tmp.emplace_back(nullptr, c);
        }
        return tmp;
    }();

    auto paths = PossiblePaths {};
    for (auto const & it : trace) {
        if (it.first == nullptr) {
            set_color(std::cout, color_t::yellow);
            std::cout << it.second;

        } else {
            auto const terminal = it.first;
            switch (next(g, paths, terminal)) {
                case result_t::failed: set_color(std::cout, color_t::red); break;
                case result_t::luck: set_color(std::cout, color_t::blue); break;
                case result_t::success:
                    set_color(std::cout, color_t::green);
                    set_style(std::cout, style_t::bold);
                    break;
            }
            // print_possible_paths(paths, printer);
            assert(check_possible_paths(paths, terminal));
            printer(terminal, std::cout);
        }
    }
    set_color(std::cout, color_t::standard);  // TODO after the loop ?
    set_style(std::cout, style_t::standard);  // TODO after the loop ?
}

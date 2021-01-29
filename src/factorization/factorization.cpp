#include "factorization.hpp"

#include <cassert>

#include "check.hpp"

#if 0
#    include <iostream>
#    define trace(v) std::cerr << v;
#else
#    define trace(v) \
        do {         \
        } while (false)
#endif

static auto constexpr nonterminal_repeats_value = std::size_t(-1);
static auto constexpr terminal_repeats_value = std::size_t(0);

Base::Base(std::size_t repeat_value) : repeats(repeat_value) {}
Symbol::Symbol(std::size_t repeat_value) : Base(repeat_value) {}
NonTerminal::NonTerminal() : Symbol(nonterminal_repeats_value) {}
Terminal::Terminal() : Symbol(terminal_repeats_value) {}

// ----------------------------------------------------------

auto is_nonterminal(Base const * n) -> bool { return n->repeats == nonterminal_repeats_value; }
auto as_nonterminal(Base * n) -> NonTerminal * {
    assert(is_nonterminal(n));
    return reinterpret_cast<NonTerminal *>(n);
}
auto as_nonterminal(Base const * n) -> NonTerminal const * {
    assert(is_nonterminal(n));
    return reinterpret_cast<NonTerminal const *>(n);
}

auto is_terminal(Base const * n) -> bool { return n->repeats == terminal_repeats_value; }
auto as_terminal(Base * n) -> Terminal * {
    assert(is_terminal(n));
    return reinterpret_cast<Terminal *>(n);
}
auto as_terminal(Base const * n) -> Terminal const * {
    assert(is_terminal(n));
    return reinterpret_cast<Terminal const *>(n);
}

auto is_symbol(Base const * n) -> bool { return is_terminal(n) || is_nonterminal(n); }
auto as_symbol(Base * n) -> Symbol * {
    assert(is_symbol(n));
    return reinterpret_cast<Symbol *>(n);
}
auto as_symbol(Base const * n) -> Symbol const * {
    assert(is_symbol(n));
    return reinterpret_cast<Symbol const *>(n);
}

static_assert(sizeof(Node) == 32);
auto is_node(Base const * n) -> bool { return !is_symbol(n); }
auto as_node(Base * n) -> Node * {
    assert(is_node(n));
    return reinterpret_cast<Node *>(n);
}
auto as_node(Base const * n) -> Node const * {
    assert(is_node(n));
    return reinterpret_cast<Node const *>(n);
}
auto is_first(Node const * n) -> bool { return is_nonterminal(n->previous); }
auto is_last(Node const * n) -> bool { return is_nonterminal(n->next); }
auto next_node(Node * n) -> Node * { return as_node(n->next); }
auto previous_node(Node * n) -> Node * { return as_node(n->previous); }

// ----------------------------------------------------------

auto NodeFactory::new_node() -> Node * {
    if (unused_nodes != nullptr) {
        auto const res = unused_nodes;
        unused_nodes = reinterpret_cast<Node *>(res->next);
        return res;
    } else {
        if (storage.back().size() == storage.back().capacity())
            new_chunk();
        storage.back().emplace_back();
        return &storage.back().back();
    }
}
auto NodeFactory::release_node(Node * n) -> void {
    n->next = unused_nodes;
    unused_nodes = n;
}

NodeFactory::NodeFactory() { new_chunk(); }

auto NodeFactory::new_chunk() -> void {
    storage.emplace_back();
    storage.back().reserve(chunk_size);
}

// ----------------------------------------------------------

auto NonTerminalFactory::new_nonterminal() -> NonTerminal * {
    if (unused_nonterminals != nullptr) {
        auto const res = unused_nonterminals;
        unused_nonterminals = reinterpret_cast<NonTerminal *>(res->first);
        return res;
    } else {
        if (storage.back().size() == storage.back().capacity())
            new_chunk();
        storage.back().emplace_back();
        return &storage.back().back();
    }
}

auto NonTerminalFactory::release_nonterminal(NonTerminal * n) -> void {
    n->last = nullptr;
    n->first = reinterpret_cast<Node *>(unused_nonterminals);
    unused_nonterminals = n;
}

auto NonTerminalFactory::in_use_nonterminals() const -> std::vector<NonTerminal const *> {
    auto res = std::vector<NonTerminal const *> {};
    res.reserve(storage.size() * chunk_size);
    for (auto const & data : storage)
        for (auto const & v : data)
            if (v.last != nullptr)
                res.push_back(&v);

    return res;
}

NonTerminalFactory::NonTerminalFactory() { new_chunk(); }

auto NonTerminalFactory::new_chunk() -> void {
    storage.emplace_back();
    storage.back().reserve(chunk_size);
}

// ----------------------------------------------------------

static auto new_node(Grammar & g) -> Node * { return g.nodes.new_node(); }
static auto remove_node(Grammar & g, Node * n) -> void { g.nodes.release_node(n); }

auto new_terminal(Grammar & g, void * payload) -> Terminal * {
    g.terminals.push_back(std::make_unique<Terminal>());
    auto const n = g.terminals.back().get();
    n->payload = payload;
    return n;
}

static auto new_nonterminal(Grammar & g) -> NonTerminal * {
    return g.nonterminals.new_nonterminal();
}

static auto remove_nonterminal(Grammar & g, NonTerminal * n) -> void {
    g.nonterminals.release_nonterminal(n);
}

// ----------------------------------------------------------

static auto get_digram(Symbol * r, Symbol * n) -> Node * {
    auto const it = r->occurences_with_successor.find(n);
    if (it == r->occurences_with_successor.end())
        return nullptr;
    return it->second;
}

static auto occurences_count(Symbol const * r) -> std::size_t {
    return r->occurences_with_successor.size() + r->occurences_without_successor.size();
}

static auto add_occurence(Symbol * rule, Node * parent, Symbol * next) -> void {
    if (next == nullptr) {
        rule->occurences_without_successor.emplace(parent);
    } else {
        rule->occurences_with_successor.emplace(next, parent);
    }
}

static auto remove_occurence(Symbol * rule, Node * parent, Symbol * next) -> void {
    if (next == nullptr) {
        rule->occurences_without_successor.extract(parent);
    } else {
        rule->occurences_with_successor.extract(next);
    }
}

static auto replace_occurence(Symbol * rule, Node * parent, Symbol * old_next, Symbol * new_next)
        -> void {
    remove_occurence(rule, parent, old_next);
    add_occurence(rule, parent, new_next);
}

static auto appendSymbol(Grammar & g, NonTerminal * r, Symbol * n) -> void {
    trace("S");
    assert(check_graph_integrity(g));
    auto p_i = r->last;  // last element of r
    auto const p = p_i->maps_to;
    if (p_i->maps_to == n) {
        trace("l");
        ++p_i->repeats;
    } else if (auto p_j = get_digram(p_i->maps_to, n); p_j != nullptr) {
        trace("p");
        auto n_k = next_node(p_j);

        // make repeats(p_i) == repeats(p_j)
        if (p_i->repeats < p_j->repeats) {
            trace("j");
            auto const p_m = new_node(g);
            p_m->maps_to = p;
            add_occurence(p, p_m, p);  // would be removed immediatly after
            p_m->repeats = p_j->repeats - p_i->repeats;
            p_j->repeats = p_i->repeats;
            if (is_first(p_j)) {
                trace("1");
                auto const r2 = as_nonterminal(p_j->previous);
                r2->first = p_m;
                p_m->previous = r2;
            } else {
                trace("2");
                auto const prev = previous_node(p_j);
                prev->next = p_m;
                p_m->previous = prev;
            }
            p_m->next = p_j;
            p_j->previous = p_m;
        } else if (p_i->repeats > p_j->repeats) {
            trace("i");
            auto const p_m = new_node(g);
            p_m->maps_to = p;
            add_occurence(p, p_m, p);
            p_m->repeats = p_i->repeats - p_j->repeats;
            p_i->repeats = p_j->repeats;
            auto const prev = previous_node(p_i);
            prev->next = p_m;
            p_m->previous = prev;
            p_m->next = p_i;
            p_i->previous = p_m;
        }

        // make repeat(n_k) == 1
        if (n_k->repeats > 1) {
            trace("k");
            // n_m is set on the left for optimization, swap n_m and n_k adresses at end of block
            auto const n_m = new_node(g);
            n_m->maps_to = n;
            add_occurence(n, n_m, n);  // would be removed immediatly after
            n_k->repeats = n_k->repeats - 1;
            n_m->repeats = 1;
            p_j->next = n_k->previous = n_m;
            n_m->previous = p_j;
            n_m->next = n_k;
            n_k = n_m;
        }

        if (!is_terminal(p) && p_i->repeats == 1 && p_j->repeats == 1 && occurences_count(p) == 2) {
            auto const r_p = as_nonterminal(p);
            // Extend a nonterminal symbol
            if (is_first(p_j) && is_last(n_k)) {
                trace("f");
                // remove p_i
                auto const prev_p_i = previous_node(p_i);
                replace_occurence(prev_p_i->maps_to, prev_p_i, p, nullptr);
                remove_occurence(p, p_i, nullptr);
                prev_p_i->next = r;
                r->last = prev_p_i;
                remove_node(g, p_i);

                auto const r_pn = as_nonterminal(p_j->previous);

                // remove r_pn content
                remove_occurence(p, p_j, n);
                remove_occurence(n, n_k, nullptr);
                remove_node(g, p_j);
                remove_node(g, n_k);

                // replace r_pn content by r_p content
                r_pn->first = r_p->first;
                r_pn->first->previous = r_pn;
                r_pn->last = r_p->last;
                r_pn->last->next = r_pn;

                remove_nonterminal(g, r_p);

                appendSymbol(g, r_pn, n);
                appendSymbol(g, r, r_pn);
            } else {
                trace("e");
                // remove p_i
                auto const prev_p_i = previous_node(p_i);
                replace_occurence(prev_p_i->maps_to, prev_p_i, p, nullptr);
                remove_occurence(p, p_i, nullptr);
                prev_p_i->next = r;
                r->last = prev_p_i;
                remove_node(g, p_i);

                if (is_last(n_k)) {
                    trace("1");
                    auto const r2 = as_nonterminal(n_k->next);
                    replace_occurence(p, p_j, n, nullptr);
                    remove_occurence(n, n_k, nullptr);
                    p_j->next = r2;
                    r2->last = p_j;
                    remove_node(g, n_k);
                } else {
                    trace("2");
                    auto const next_n_k = next_node(n_k);
                    replace_occurence(p, p_j, n, next_n_k->maps_to);
                    remove_occurence(n, n_k, next_n_k->maps_to);
                    p_j->next = next_n_k;
                    next_n_k->previous = p_j;
                    remove_node(g, n_k);
                }

                appendSymbol(g, r_p, n);
                appendSymbol(g, r, p);
            }
        } else if (is_first(p_j) && is_last(n_k)) {
            trace("r");
            // remove p_i
            auto const prev_p_i = previous_node(p_i);
            replace_occurence(prev_p_i->maps_to, prev_p_i, p, nullptr);
            remove_occurence(p, p_i, nullptr);
            prev_p_i->next = r;
            r->last = prev_p_i;
            remove_node(g, p_i);

            // Reuse au nonterminal symbol
            auto const s = as_nonterminal(p_j->previous);

            // add s at end of r
            appendSymbol(g, r, s);
        } else {
            trace("c");
            // remove p_i
            auto const prev_p_i = previous_node(p_i);
            replace_occurence(prev_p_i->maps_to, prev_p_i, p, nullptr);
            remove_occurence(p, p_i, nullptr);
            prev_p_i->next = r;
            r->last = prev_p_i;

            // Create a new nonterminal symbol
            auto s = new_nonterminal(g);
            p_i->maps_to = s;
            p_i->repeats = 1;

            // remplace p_j n_k by s (reuse p_i as s_0)
            //   left side
            if (is_first(p_j)) {
                trace("1");
                auto const r2 = as_nonterminal(p_j->previous);
                r2->first = p_i;
                p_i->previous = r2;
            } else {
                trace("2");
                auto const prev_p_j = previous_node(p_j);
                replace_occurence(prev_p_j->maps_to, prev_p_j, p, s);
                prev_p_j->next = p_i;
                p_i->previous = prev_p_j;
            }
            s->first = p_j;
            p_j->previous = s;

            //   right side
            if (is_last(n_k)) {
                trace("3");
                auto const r2 = as_nonterminal(n_k->next);
                add_occurence(s, p_i, nullptr);
                r2->last = p_i;
                p_i->next = r2;
            } else {
                trace("4");
                auto const next_n_k = next_node(n_k);
                replace_occurence(n, n_k, next_n_k->maps_to, nullptr);
                add_occurence(s, p_i, next_n_k->maps_to);
                next_n_k->previous = p_i;
                p_i->next = next_n_k;
            }
            s->last = n_k;
            n_k->next = s;

            // add s at end of r
            appendSymbol(g, r, s);
        }
    } else {
        trace("a");
        // add with no simplification
        remove_occurence(p_i->maps_to, p_i, nullptr);
        p_i->next = r->last = new_node(g);
        p_i->next->repeats = 1;
        r->last->next = r;
        r->last->previous = p_i;
        r->last->maps_to = n;
        add_occurence(p_i->maps_to, p_i, n);
        add_occurence(n, r->last, nullptr);
    }
}

auto insertSymbol(Grammar & g, NonTerminal * nt, Terminal * t) -> NonTerminal * {
    assert(is_terminal(t));
    if (nt == nullptr) {
        trace("\nI");
        nt = new_nonterminal(g);
        nt->first = nt->last = new_node(g);
        nt->first->repeats = 1;
        nt->first->next = nt->first->previous = nt;
        nt->first->maps_to = t;
        add_occurence(t, nt->first, nullptr);
    } else {
        trace("I");
        appendSymbol(g, nt, t);
    }
    return nt;
}


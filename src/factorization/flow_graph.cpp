#include "flow_graph.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "reduction.hpp"

static auto getSymbolOfNode(GrammarNode const * n) -> Symbol const * {
    while (is_node(n->next))
        n = as_node(n->next);
    return as_symbol(n->next);
}

static auto getOcurenceCount(Symbol const * symbol) -> size_t {
    auto res = size_t(0);
    for (auto const & parent : symbol->occurrences_without_successor)
        res += getOcurenceCount(getSymbolOfNode(parent)) * parent->repeats;
    for (auto const & [next, parent] : symbol->occurrences_with_successor)
        res += getOcurenceCount(getSymbolOfNode(parent)) * parent->repeats;
    if (res == 0)
        return 1;
    return res;
}

static auto getFirstTerminal(Symbol const * s) -> Terminal const * {
    while (is_nonterminal(s))
        s = as_nonterminal(s)->first->maps_to;
    return as_terminal(s);
}

// -------------------------------------------------------------

auto FlowGraph::build_from(Grammar const & g) -> void {
    auto node_map = std::unordered_map<GrammarBaseObject const *, FlowNode *> {};
    auto get_node = [&node_map, this](GrammarBaseObject const * obj) -> FlowNode * {
        assert(is_node(obj) || is_terminal(obj));
        auto const [it, inserted] = node_map.emplace(obj, nullptr);
        if (inserted)
            it->second = new_node();
        return it->second;
    };

    // Create terminals
    for (auto const & terminal : g.terminals) {
        auto const node = get_node(terminal.get());
        node->first_terminal = terminal.get();
        node->next = nullptr;
        node->repeats = 1;
        node->count = getOcurenceCount(terminal.get());
        node->son = nullptr;
        entry_points.emplace(terminal.get(), node);
    }

    // Create non_terminals
    for (auto const & non_terminal : g.nonterminals.in_use_nonterminals()) {
        auto grammar_node = non_terminal->first;
        auto const count = getOcurenceCount(non_terminal);
        while (true) {
            auto const node = get_node(grammar_node);
            node->first_terminal = getFirstTerminal(grammar_node->maps_to);
            node->repeats = grammar_node->repeats;
            node->count = count;

            if (is_terminal(grammar_node->maps_to))
                node->son = get_node(grammar_node->maps_to);
            else
                node->son = get_node(as_nonterminal(grammar_node->maps_to)->first);

            if (is_node(grammar_node->next)) {
                node->next = get_node(grammar_node->next);

                grammar_node = as_node(grammar_node->next);
            } else {
                node->next = nullptr;
                break;
            }
        }
    }

    // Create transitions
    for (auto const & non_terminal : g.nonterminals.in_use_nonterminals()) {
        auto node = get_node(non_terminal->first);

        while (node != nullptr) {
            auto son = node->son;

            auto depth = size_t(1);
            while (son != nullptr) {
                while (son->next != nullptr)
                    son = son->next;

                if (node->next != nullptr)
                    son->transitions.emplace_back(Transition {
                            node->next,
                            depth,
                    });

                if (node->repeats > 1)
                    son->transitions.emplace_back(Transition {
                            node,
                            depth,
                    });

                ++depth;
                son = son->son;
            }

            node = node->next;
        }
    }
}

auto FlowGraph::load_from(std::istream & is) -> void {
    // TODO
    assert(false);
}

auto FlowGraph::save_to(std::ostream & os) -> void {
    // TODO
    assert(false);
}

auto FlowGraph::get_entry_point(Terminal const * terminal) const -> FlowNode const * {
    auto const it = entry_points.find(terminal);
    if (it != entry_points.end())
        return it->second;
    return nullptr;
}

auto FlowGraph::new_node() -> FlowNode * {
    nodes.emplace_back(std::make_unique<FlowNode>());
    return nodes.back().get();
}


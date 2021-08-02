#include "flow_graph.hpp"

#include <cassert>
#include <iostream>  // TODO remove

static auto getSymbolOfNode(GrammarNode const * n) -> Symbol const * {
    while (is_node(n->next))
        n = as_node(n->next);
    return as_symbol(n->next);
}

static auto getOcurenceCount(GrammarNode const * node) -> size_t {
    auto const symbol = getSymbolOfNode(node);
    auto res = size_t(0);
    for (auto const & parent : symbol->occurrences_without_successor)
        res += getOcurenceCount(parent) * parent->repeats;
    for (auto const & [next, parent] : symbol->occurrences_with_successor)
        res += getOcurenceCount(parent) * parent->repeats;
    if (res == 0)
        return 1;
    return res;
}

static auto getFirstTerminal(Symbol const * s) -> Terminal const * {
    while (is_nonterminal(s))
        s = as_nonterminal(s)->first->maps_to;
    return as_terminal(s);
}

static auto getNodeIndex(GrammarNode const * n) -> size_t {
    auto res = 0u;
    while (is_node(n->previous)) {
        ++res;
        n = as_node(n->previous);
    }
    return res;
}

static auto addTransition(std::vector<Transition> * out,
                          std::vector<std::unique_ptr<FakePattern>> & fake_patterns,
                          PatternBase * pattern,
                          GrammarNode const * node,
                          size_t pop_count,
                          size_t ocurrences_count) -> void {
    auto const first_terminal = getFirstTerminal(node->maps_to);
    auto const node_index = getNodeIndex(node);

    auto transition = [&]() -> Transition * {
        for (auto & transition : *out) {
            if (transition.terminal == first_terminal) {
                return &transition;
            }
        }
        return nullptr;
    }();

    if (transition == nullptr) {
        out->push_back(
                Transition { first_terminal, pattern, node_index, pop_count, ocurrences_count });
    } else {
        auto const fake_pattern = [&]() {
            if (is_fake_pattern(transition->pattern)) {
                return as_fake_pattern(transition->pattern);
            } else {
                fake_patterns.push_back(std::make_unique<FakePattern>());
                auto const ptr = fake_patterns.back().get();
                ptr->patterns.push_back(FakePatternOccurence { as_pattern(transition->pattern),
                                                               transition->node_index });
                transition->node_index = -1;
                transition->pattern = ptr;
                ptr->terminal = first_terminal;
                return ptr;
            }
        }();

        transition->ocurence_count += ocurrences_count;
        fake_pattern->patterns.push_back(FakePatternOccurence { pattern, node_index });
    }
}

static auto exploreTransitions(std::vector<Transition> * out,
                               std::vector<std::unique_ptr<FakePattern>> & fake_patterns,
                               std::unordered_map<Symbol const *, Pattern *> const & patterns,
                               Symbol const * s,
                               size_t pop_count) -> void {
    assert(out != nullptr);

    for (auto const & [next, parent] : s->occurrences_with_successor) {
        auto const symbol_pattern = patterns.at(getSymbolOfNode(parent));

        auto const ocurrences_count = getOcurenceCount(parent);
        if (parent->repeats > 1)
            addTransition(out,
                          fake_patterns,
                          symbol_pattern,
                          parent,
                          pop_count,
                          (parent->repeats - 1) * ocurrences_count);

        addTransition(out,
                      fake_patterns,
                      symbol_pattern,
                      as_node(parent->next),
                      pop_count,
                      ocurrences_count);
    }

    for (auto const & parent : s->occurrences_without_successor) {
        auto const symbol_pattern = patterns.at(getSymbolOfNode(parent));
        if (parent->repeats > 1) {
            auto const ocurrences_count = getOcurenceCount(parent);
            addTransition(out, fake_patterns, symbol_pattern, parent, pop_count, ocurrences_count);
        }
        exploreTransitions(out, fake_patterns, patterns, getSymbolOfNode(parent), pop_count + 1);
    }
}

auto buildFlowGraph(Grammar & g) -> FlowGraph {
    auto const non_terminals = g.nonterminals.in_use_nonterminals();
    auto const pattern_count = g.terminals.size() + non_terminals.size();

    // true patterns are stable in memory
    auto patterns = std::unordered_map<Symbol const *, Pattern *> {};
    patterns.reserve(pattern_count);

    auto graph = FlowGraph {};
    graph.patterns.resize(pattern_count);
    graph.terminals_index.reserve(g.terminals.size());

    for (auto i = 0u; i < g.terminals.size(); ++i) {
        auto const terminal = g.terminals[i].get();
        auto const pattern = &graph.patterns[i];
        pattern->symbol = terminal;
        patterns[pattern->symbol] = pattern;
        // assert(terminal->pattern == nullptr); // TODO
        terminal->pattern = pattern;
    }

    for (auto i = 0u; i < non_terminals.size(); ++i) {
        auto const index = i + g.terminals.size();
        auto & pattern = graph.patterns[index];
        pattern.symbol = non_terminals[i];
        patterns[pattern.symbol] = &pattern;

        if (pattern.symbol->occurrences_with_successor.size() +
                    pattern.symbol->occurrences_without_successor.size() ==
            0)

            graph.roots.push_back(&pattern);
    }

    for (auto & pattern : graph.patterns) {
        if (is_nonterminal(pattern.symbol)) {
            auto node = as_nonterminal(pattern.symbol)->first;
            while (true) {
                pattern.nodes.push_back(
                        PatternNode { node, patterns.at(node->maps_to), node->repeats });
                if (!is_node(node->next))
                    break;
                node = as_node(node->next);
            }
        }
        exploreTransitions(&pattern.transitions, graph.fake_patterns, patterns, pattern.symbol, 1);
    }

    auto const initial_fake_pattern_count = graph.fake_patterns.size();
    for (auto i = 0u; i < graph.fake_patterns.size(); ++i) {
        if (i > initial_fake_pattern_count) {
            std::cerr << "FAKE PATTERN GENERATION FAILURE" << std::endl;
            break;
        }

        auto const fake_pattern = graph.fake_patterns[i].get();
        auto total_occurence_count = 0u;
        for (auto const & node : fake_pattern->patterns) {
            auto const pattern = as_pattern(node.pattern);
            auto const & current_node = pattern->nodes[node.node_index];
            auto const ocurrences_count = getOcurenceCount(current_node.node);
            total_occurence_count += ocurrences_count;

            if (fake_pattern->count == 0 || fake_pattern->count > current_node.node->repeats)
                fake_pattern->count = current_node.node->repeats;

            auto const next_node_index = node.node_index + 1;
            if (next_node_index < pattern->nodes.size()) {
                addTransition(&fake_pattern->transitions,
                              graph.fake_patterns,
                              node.pattern,
                              pattern->nodes[next_node_index].node,
                              1,  // TODO
                              ocurrences_count);
            } else {
                exploreTransitions(&fake_pattern->transitions,
                                   graph.fake_patterns,
                                   patterns,
                                   pattern->symbol,
                                   1);  // TODO
            }
        }

        if (fake_pattern->count > 1)
            fake_pattern->transitions.push_back(Transition { fake_pattern->terminal,
                                                             fake_pattern,
                                                             size_t(-1),
                                                             0,
                                                             total_occurence_count });
    }

    // TODO debug remove

    auto const is_transition_valid = [](Transition const & transition) -> bool {
        if (is_fake_pattern(transition.pattern)) {
            auto const fake_pattern = as_fake_pattern(transition.pattern);
            if (fake_pattern->patterns.size() < 2)
                return false;

            // TODO
            // for (auto const [pattern, node_index] : fake_pattern->patterns) {
            //     auto const node = as_pattern(pattern)->nodes[node_index].node;
            //     if (getFirstTerminal(node->maps_to) != transition.terminal)
            //         return false;
            // }

            return true;
        } else {
            auto const pattern =
                    as_pattern(transition.pattern)->nodes[transition.node_index].pattern;
            return getFirstTerminal(pattern->symbol) == transition.terminal;
        }
    };

    for (auto const & fake_pattern : graph.fake_patterns) {
        for (auto const & transition : fake_pattern->transitions)
            assert(is_transition_valid(transition));
    }

    for (auto const & pattern : graph.patterns) {
        for (auto const & transition : pattern.transitions)
            assert(is_transition_valid(transition));
    }

    // TODO /debug

    return graph;
}


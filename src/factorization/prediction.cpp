#include "prediction.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>  // TODO remove

static auto getFirstTerminal(Symbol const * s) -> Terminal const * {
    while (is_nonterminal(s))
        s = as_nonterminal(s)->first->maps_to;
    return as_terminal(s);
}

static auto getSymbolOfNode(GrammarNode const * n) -> Symbol const * {
    while (is_node(n->next))
        n = as_node(n->next);
    return as_symbol(n->next);
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
                          size_t pop_count) -> void {
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
        out->push_back(Transition { first_terminal, pattern, node_index, pop_count });
    } else {
        auto const fake_pattern = [&]() {
            if (is_fake_pattern(transition->pattern)) {
                return as_fake_pattern(transition->pattern);
            } else {
                std::cerr << "Add fake pattern\n";
                fake_patterns.push_back(std::make_unique<FakePattern>());
                auto const ptr = fake_patterns.back().get();
                ptr->patterns.push_back(FakePatternOccurence { as_pattern(transition->pattern),
                                                               transition->node_index });
                transition->node_index = -1;
                transition->pattern = ptr;
                return ptr;
            }
        }();

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

        if (parent->repeats > 1)
            addTransition(out, fake_patterns, symbol_pattern, parent, pop_count);

        addTransition(out, fake_patterns, symbol_pattern, as_node(parent->next), pop_count);
    }

    for (auto const & parent : s->occurrences_without_successor) {
        auto const symbol_pattern = patterns.at(getSymbolOfNode(parent));
        if (parent->repeats > 1)
            addTransition(out, fake_patterns, symbol_pattern, parent, pop_count);
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
        assert(terminal->pattern == nullptr);
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

    std::cerr << graph.fake_patterns.size() << " fake patterns were built.\n";

    for (auto i = 0u; i < graph.fake_patterns.size(); ++i) {
        if (i > 10) {
            std::cerr << "ERROR recursivity" << std::endl;
            break;
        }
        auto const fake_pattern = graph.fake_patterns[i].get();
        std::cerr << "Search for transitions after fake_node : ";
        for (auto const & node : fake_pattern->patterns) {
            std::cerr << 0 << std::endl;
            auto const pattern = as_pattern(node.pattern);
            auto const & current_node = pattern->nodes[node.node_index];
            if (current_node.node->repeats > 1) {
                std::cerr << 1 << std::endl;
                addTransition(&fake_pattern->transitions,
                              graph.fake_patterns,
                              pattern,
                              current_node.node,
                              1);  // TODO
            }

            auto const next_node_index = node.node_index + 1;
            if (next_node_index < pattern->nodes.size()) {
                std::cerr << 2 << std::endl;
                addTransition(&fake_pattern->transitions,
                              graph.fake_patterns,
                              node.pattern,
                              current_node.node,
                              1);  // TODO
            } else {
                std::cerr << 3 << std::endl;
                exploreTransitions(&fake_pattern->transitions,
                                   graph.fake_patterns,
                                   patterns,
                                   pattern->symbol,
                                   1);  // TODO
            }
        }
        std::cerr << "found " << fake_pattern->transitions.size() << std::endl;
    }

    // TODO debug remove

    auto const is_transition_valid = [](Transition const & transition) -> bool {
        if (is_fake_pattern(transition.pattern)) {
            assert(false);  // TODO
            // for (auto const pattern : as_fake_pattern(transition.pattern)->patterns)
            //     if (getFirstTerminal(pattern.pattern->nodes[transition.node_index].symbol) !=
            //         transition.terminal)
            //         return false;
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

// ----------------------------------------------------------------
// Usefull for both estimation and prediction
// ----------------------------------------------------------------

static auto descend(Estimation & e) -> void {
    if (!is_fake_pattern(e.back().pattern)) {
        while (true) {
            auto const pattern = as_pattern(e.back().pattern);

            if (is_terminal(pattern->symbol))
                break;

            e.emplace_back(EstimationNode { pattern->nodes[e.back().node_index].pattern, 0 });
        }
    }
}

// ----------------------------------------------------------------
// Estimation
// ----------------------------------------------------------------

auto init_estimation(Estimation * e, FlowGraph const * g) -> void {
    assert(e != nullptr);
    assert(g != nullptr);
    e->clear();
}

// ----------------------------------------------------------------

auto update_estimation(Estimation * e, Terminal const * t) -> void {
    assert(e != nullptr);
    assert(t != nullptr);
    assert(t->pattern != nullptr);

    if (e->size() > 0) {
        for (auto const transition : e->back().pattern->transitions) {
            if (transition.terminal == t) {
                if (transition.pop_count < e->size()) {
                    e->resize(e->size() - transition.pop_count);
                    assert(e->back().pattern == transition.pattern);
                    e->back().node_index = transition.node_index;
                } else {
                    e->clear();
                    e->emplace_back(EstimationNode { transition.pattern, transition.node_index });
                }
                descend(*e);
                assert(as_pattern(e->back().pattern)->symbol == t);
                assert(e->back().pattern == t->pattern);
                return;
            }
        }
        e->clear();
    }

    e->emplace_back(EstimationNode { t->pattern, 0 });
}

// ----------------------------------------------------------------

auto deinit_estimation(Estimation * e) -> void { e->clear(); }

// ----------------------------------------------------------------
// Prediction
// ----------------------------------------------------------------

static auto skip_false_prediction(Prediction * p) -> bool {
    assert(p != nullptr);
    while (true) {
        auto const & transitions = p->estimation.back().pattern->transitions;

        if (p->transition_index >= transitions.size())
            return false;

        auto const & transition = transitions[p->transition_index];

        if (p->estimation.size() <= transition.pop_count)
            return true;

        if (p->estimation[p->estimation.size() - transition.pop_count].pattern ==
            transition.pattern)
            return true;  // TODO Bug ?

        ++p->transition_index;
    }
}

auto reset_prediction(Prediction * p, Estimation const * e) -> bool {
    assert(p != nullptr);
    assert(e != nullptr);

    p->transition_index = 0;
    if (e->size() == 0) {
        p->estimation.clear();
        return false;
    }
    p->estimation = *e;
    return skip_false_prediction(p);
}

auto copy_prediction(Prediction * to, Prediction const * from) -> void {
    assert(to != nullptr);
    assert(from != nullptr);
    *to = *from;
}

auto get_prediction_tree_sibling(Prediction * p) -> bool {
    assert(p != nullptr);
    ++p->transition_index;
    return skip_false_prediction(p);
}

auto get_prediction_tree_child(Prediction * p) -> bool {
    assert(p != nullptr);

    auto const transition = p->estimation.back().pattern->transitions[p->transition_index];
    if (is_fake_pattern(transition.pattern)) {
        assert(false);  // TODO
    } else {
        if (p->estimation.size() > transition.pop_count) {
            p->estimation.resize(p->estimation.size() - transition.pop_count);
            assert(p->estimation.back().pattern == transition.pattern);
            p->estimation.back().node_index = transition.node_index;
        } else {
            p->estimation.clear();
            p->estimation.emplace_back(
                    EstimationNode { transition.pattern, transition.node_index });
        }
        descend(p->estimation);
        return false;  // TODO
    }
}

auto get_terminal(Prediction const * p) -> Terminal const * {
    assert(p != nullptr);
    assert(p->estimation.size() > 0);
    assert(p->estimation.back().pattern != nullptr);
    assert(p->estimation.back().pattern->transitions.size() > p->transition_index);
    return p->estimation.back().pattern->transitions[p->transition_index].terminal;
}

auto get_count(Prediction const * p) -> size_t {
    // return p->estimation.back().pattern->transitions[p->transition_index].
    return 1;
}

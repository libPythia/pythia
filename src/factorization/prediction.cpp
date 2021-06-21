#include "prediction.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>   // TODO remove
#include <iostream>  // TODO remove

static auto getLogOutput() -> std::ostream & {
    static auto log_output = std::ofstream { "log_output.txt" };
    return log_output;
}

template <typename... T> static auto log(T &&... args) {
    auto & os = getLogOutput();
    ((os << args), ...);
    os << std::endl;
}

static auto operator<<(std::ostream & os, Estimation const & e) -> std::ostream & {
    auto first = true;
    os << '{';
    for (auto const & n : e) {
        if (first)
            first = false;
        else
            os << ", ";
        os << n.pattern;
        if (!is_fake_pattern(n.pattern)) {
            auto const symbol = as_pattern(n.pattern)->symbol;
            if (is_terminal(symbol))
                os << '[' << (char const *)as_terminal(symbol)->payload << ']';
        }
    }
    os << '}';

    return os;
}

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

static auto addTransition(std::vector<Transition> * out,
                          std::vector<std::unique_ptr<FakePattern>> & fake_patterns,
                          PatternBase * pattern,
                          GrammarNode const * node,
                          size_t pop_count,
                          size_t ocurrences_count) -> void {
    auto const first_terminal = getFirstTerminal(node->maps_to);
    auto const node_index = getNodeIndex(node);
    log("add transition");

    auto transition = [&]() -> Transition * {
        for (auto & transition : *out) {
            if (transition.terminal == first_terminal) {
                return &transition;
            }
        }
        return nullptr;
    }();

    if (transition == nullptr) {
        log("Create a new transition");
        out->push_back(
                Transition { first_terminal, pattern, node_index, pop_count, ocurrences_count });
    } else {
        auto const fake_pattern = [&]() {
            if (is_fake_pattern(transition->pattern)) {
                log("Reuse transition in fake pattern");
                return as_fake_pattern(transition->pattern);
            } else {
                log("Add fake pattern");
                fake_patterns.push_back(std::make_unique<FakePattern>());
                auto const ptr = fake_patterns.back().get();
                ptr->patterns.push_back(FakePatternOccurence { as_pattern(transition->pattern),
                                                               transition->node_index });
                transition->node_index = -1;
                transition->pattern = ptr;
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
        log("build terminal pattern");
        auto const terminal = g.terminals[i].get();
        auto const pattern = &graph.patterns[i];
        pattern->symbol = terminal;
        patterns[pattern->symbol] = pattern;
        // assert(terminal->pattern == nullptr); // TODO
        terminal->pattern = pattern;
    }

    for (auto i = 0u; i < non_terminals.size(); ++i) {
        log("build non terminal pattern");
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

    log(graph.fake_patterns.size(),
        " fake patterns were built. Compute transitions after fake patterns");

    for (auto i = 0u; i < graph.fake_patterns.size(); ++i) {
        if (i > 10) {
            log("ERROR recursivity");
            break;
        }
        log("compute transitions of fake pattern");
        auto const fake_pattern = graph.fake_patterns[i].get();
        for (auto const & node : fake_pattern->patterns) {
            auto const pattern = as_pattern(node.pattern);
            auto const & current_node = pattern->nodes[node.node_index];
            auto const ocurrences_count = getOcurenceCount(current_node.node);
            if (current_node.node->repeats > 1) {
                log("Add loop transition");
                addTransition(&fake_pattern->transitions,
                              graph.fake_patterns,
                              pattern,
                              current_node.node,
                              1,
                              (current_node.node->repeats - 1) * ocurrences_count);  // TODO
            }

            auto const next_node_index = node.node_index + 1;
            if (next_node_index < pattern->nodes.size()) {
                log("Add transition to next node in pattern");
                addTransition(&fake_pattern->transitions,
                              graph.fake_patterns,
                              node.pattern,
                              pattern->nodes[next_node_index].node,
                              1,  // TODO
                              ocurrences_count);
            } else {
                log("Explore transitions after pattern end");
                exploreTransitions(&fake_pattern->transitions,
                                   graph.fake_patterns,
                                   patterns,
                                   pattern->symbol,
                                   1);  // TODO
            }
        }
    }

    // TODO debug remove

    auto const is_transition_valid = [](Transition const & transition) -> bool {
        if (is_fake_pattern(transition.pattern)) {
            auto const fake_pattern = as_fake_pattern(transition.pattern);
            if (fake_pattern->patterns.size() < 2) {
                log("ERROR too many nodes represented by a fake pattern");
            }
            for (auto const [pattern, node_index] : fake_pattern->patterns) {
                auto const node = as_pattern(pattern)->nodes[node_index].node;
                if (getFirstTerminal(node->maps_to) != transition.terminal) {
                    log("Wrong transition over fake node");
                    return false;
                }
            }
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

            log("descend");
            e.emplace_back(EstimationNode { pattern->nodes[e.back().node_index].pattern, 0 });
        }
    }
}

// ----------------------------------------------------------------
// Estimation
// ----------------------------------------------------------------

auto init_estimation(Estimation * e, FlowGraph const * g) -> void {
    // log("init estimation");
    assert(e != nullptr);
    assert(g != nullptr);
    e->clear();
}

// ----------------------------------------------------------------

auto update_estimation(Estimation * e, Terminal const * t) -> void {
    assert(e != nullptr);

    if (t == nullptr) {
        log("update estimation : unknown event");
        e->clear();
        return;
    }

    log("update estimation : ",
        (char const *)t->payload,
        "-----------------------------------------------------");

    assert(t != nullptr);
    assert(t->pattern != nullptr);

    if (e->size() > 0) {
        for (auto const transition : e->back().pattern->transitions) {
            if (transition.terminal == t) {
                if (transition.pop_count < e->size()) {
                    log("update forward");
                    e->resize(e->size() - transition.pop_count);
                    assert(e->back().pattern == transition.pattern);
                    e->back().node_index = transition.node_index;
                } else {
                    log("update climb");
                    e->clear();
                    e->emplace_back(EstimationNode { transition.pattern, transition.node_index });
                }
                descend(*e);
                assert(as_pattern(e->back().pattern)->symbol == t);
                assert(e->back().pattern == t->pattern);
                log("update : ", *e);
                return;
            }
        }
        e->clear();
    }

    e->emplace_back(EstimationNode { t->pattern, 0 });
    log("update init : ", *e);
    return;
}

// ----------------------------------------------------------------

auto deinit_estimation(Estimation * e) -> void {
    // log("deinit estimation");
    e->clear();
}

// ----------------------------------------------------------------
// Prediction
// ----------------------------------------------------------------

static auto skip_false_prediction(Prediction * p) -> bool {
    assert(p != nullptr);
    while (true) {
        auto const & transitions = p->estimation.back().pattern->transitions;

        if (p->transition_index >= transitions.size()) {
            log("end of predictions");
            return false;
        }

        auto const & transition = transitions[p->transition_index];
        log("Inspect transition : ", transition.pop_count, " -> ", transition.pattern);

        if (p->estimation.size() <= transition.pop_count) {
            log("possible climbing");
            return true;
        }

        auto const pattern_index = p->estimation.size() - transition.pop_count - 1;
        if (p->estimation[pattern_index].pattern == transition.pattern) {
            auto const node_index = p->estimation[pattern_index].node_index;
            if (transition.node_index == node_index || transition.node_index == node_index + 1) {
                log("possible transition");
                return true;
            }
        }

        log("skip transition");

        ++p->transition_index;
    }
}

auto reset_prediction(Prediction * p, Estimation const * e) -> bool {
    log("reset prediction");
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
    log("copy prediction : ", from->estimation);
    assert(to != nullptr);
    assert(from != nullptr);
    *to = *from;
}

auto get_prediction_tree_sibling(Prediction * p) -> bool {
    assert(p != nullptr);
    ++p->transition_index;
    log("get sibling : ", p->estimation);
    return skip_false_prediction(p);
}

auto get_prediction_tree_child(Prediction * p) -> bool {
    assert(p != nullptr);

    auto const transition = p->estimation.back().pattern->transitions[p->transition_index];
    if (is_fake_pattern(transition.pattern)) {
        assert(false);  // TODO
    } else {
        if (transition.pop_count < p->estimation.size()) {
            log("get child : prediction forward");
            p->estimation.resize(p->estimation.size() - transition.pop_count);
            assert(p->estimation.back().pattern == transition.pattern);
            // assert(p->estimation.back().node_index == transition.node_index ||
            //        p->estimation.back().node_index == transition.node_index + 1); // TODO
            p->estimation.back().node_index = transition.node_index;
        } else {
            log("get child : prediction climb");
            p->estimation.clear();
            p->estimation.emplace_back(
                    EstimationNode { transition.pattern, transition.node_index });
        }
        descend(p->estimation);
        p->transition_index = 0u;
        log("get child : ", p->estimation);
        return skip_false_prediction(p);
    }
}

auto get_terminal(Prediction const * p) -> Terminal const * {
    assert(p != nullptr);
    assert(p->estimation.size() > 0);
    assert(p->estimation.back().pattern != nullptr);
    assert(p->estimation.back().pattern->transitions.size() > p->transition_index);
    log("get terminal ",
        (char const *)p->estimation.back()
                .pattern->transitions[p->transition_index]
                .terminal->payload);
    return p->estimation.back().pattern->transitions[p->transition_index].terminal;
}

auto get_count(Prediction const * p) -> size_t {
    return p->estimation.back().pattern->transitions[p->transition_index].ocurence_count;
}

auto get_probability(Prediction const * p) -> double {
    auto count = size_t(0u);

    for (auto const & transition : p->estimation.back().pattern->transitions)
        count += transition.ocurence_count;

    auto const proba = static_cast<double>(get_count(p)) / static_cast<double>(count);
    // assert(proba >= 0. && proba <= 1.);
    return proba;
}


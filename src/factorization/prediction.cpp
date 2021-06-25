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
            os << 'x' << n.repeat;
        }
    }
    os << '}';

    return os;
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

            e.emplace_back(EstimationNode { pattern->nodes[e.back().node_index].pattern, 0, 1 });
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

auto update_estimation(Estimation * estimation, Terminal const * terminal) -> void {
    assert(estimation != nullptr);

    if (terminal == nullptr) {
        estimation->clear();
        return;
    }

    log("update estimation : ",
        (char const *)terminal->payload,
        "-----------------------------------------------------");

    assert(terminal != nullptr);
    assert(terminal->pattern != nullptr);

    if (estimation->size() > 0) {
        for (auto const transition : estimation->back().pattern->transitions) {
            if (transition.terminal == terminal) {
                if (transition.pop_count < estimation->size()) {
                    estimation->resize(estimation->size() - transition.pop_count);
                    assert(estimation->back().pattern == transition.pattern);
                    if (estimation->back().node_index == transition.node_index) {
                        assert(estimation->back().repeat > 0);

                        ++estimation->back().repeat;

                        // Detect too much loop iterations
                        auto const node = as_pattern(estimation->back().pattern)
                                                  ->nodes[estimation->back().node_index];
                        if (estimation->back().repeat == node.count) {
                            break;
                        }
                    } else {
                        estimation->back().node_index = transition.node_index;
                        estimation->back().repeat = 1;
                    }
                } else {
                    auto const repeat =
                            (as_pattern(transition.pattern)->nodes[transition.node_index].pattern ==
                             estimation->front().pattern)
                                    ? size_t(2)
                                    : size_t(1);
                    estimation->clear();
                    estimation->emplace_back(
                            EstimationNode { transition.pattern, transition.node_index, repeat });
                }
                descend(*estimation);
                assert(as_pattern(estimation->back().pattern)->symbol == terminal);
                assert(estimation->back().pattern == terminal->pattern);
                return;
            }
        }
        estimation->clear();
    }

    estimation->emplace_back(EstimationNode { terminal->pattern, 0, 1 });
    return;
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

        if (p->transition_index >= transitions.size()) {
            return false;
        }

        auto const & transition = transitions[p->transition_index];

        if (p->estimation.size() <= transition.pop_count) {
            return true;
        }

        auto const pattern_index = p->estimation.size() - transition.pop_count - 1;
        if (p->estimation[pattern_index].pattern == transition.pattern) {
            auto const node_index = p->estimation[pattern_index].node_index;
            if (transition.node_index == node_index || transition.node_index == node_index + 1) {
                return true;
            }
        }

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
        if (transition.pop_count < p->estimation.size()) {
            p->estimation.resize(p->estimation.size() - transition.pop_count);
            assert(p->estimation.back().pattern == transition.pattern);
            if (p->estimation.back().node_index == transition.node_index)
                ++p->estimation.back().repeat;
            else {
                p->estimation.back().node_index = transition.node_index;
                p->estimation.back().repeat = 1;
            }
        } else {
            auto const repeat =
                    (as_pattern(transition.pattern)->nodes[transition.node_index].pattern ==
                     p->estimation.front().pattern)
                            ? size_t(2)
                            : size_t(1);
            p->estimation.clear();
            p->estimation.emplace_back(
                    EstimationNode { transition.pattern, transition.node_index, repeat });
        }
        descend(p->estimation);
        p->transition_index = 0u;
        return skip_false_prediction(p);
    }
}

static auto get_current_transition(Prediction const * p) -> Transition const * {
    assert(p != nullptr);
    assert(p->estimation.size() > 0);
    assert(p->estimation.back().pattern != nullptr);
    assert(p->estimation.back().pattern->transitions.size() > p->transition_index);
    return &p->estimation.back().pattern->transitions[p->transition_index];
}

auto get_terminal(Prediction const * p) -> Terminal const * {
    auto const transition = get_current_transition(p);
    return transition->terminal;
}

static auto compute_transition_occurence_count(Prediction const * p, size_t const index) -> size_t {
    auto const & transition = p->estimation.back().pattern->transitions[index];
    auto const repeat_count = [&]() -> size_t {
        if (p->estimation.size() > transition.pop_count) {
            auto const & target = p->estimation[p->estimation.size() - transition.pop_count - 1];
            assert(target.pattern == transition.pattern);
            if (target.node_index == transition.node_index) {
                return target.repeat - 1;
            }
        }
        return 0;
    }();
    return transition.ocurence_count - repeat_count;
}

auto get_probability(Prediction const * p) -> double {
    auto const current_transition = get_current_transition(p);

    auto const proba = [&]() {
        if (p->estimation.size() < current_transition->pop_count) {
            return 1.;
        }

        auto count = size_t(0u);
        auto total_count = size_t(0u);

        auto const transition_count = p->estimation.back().pattern->transitions.size();
        for (auto transition_index = 0u; transition_index < transition_count; ++transition_index) {
            auto const transition_count = compute_transition_occurence_count(p, transition_index);
            total_count += transition_count;
            if (transition_index == p->transition_index)
                count = transition_count;
        }

        return static_cast<double>(count) / static_cast<double>(total_count);
    }();
    // assert(proba >= 0. && proba <= 1.);
    return proba;
}


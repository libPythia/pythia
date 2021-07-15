#include "prediction.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>   // TODO remove
#include <iostream>  // TODO remove

static auto getLogOutput() -> std::ostream & {
    static auto log_output = std::ofstream { "log_output.txt" };
    return log_output;
}
static int disable_logging = 0;

struct log_lock {
    log_lock() { ++disable_logging; }
    ~log_lock() { --disable_logging; }
};

template <typename... T> static auto log(T &&... args) {
    if (disable_logging > 0) {
        auto & os = getLogOutput();
        ((os << args), ...);
        os << std::endl;
    }
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

            e.emplace_back(EstimationNode { pattern->nodes[e.back().node_index].pattern,
                                            0,
                                            1,
                                            LoopEstimation::from_start });
        }
    }
}

static auto get_estimation_child(Estimation * estimation, Transition const & transition) -> bool {
    if (transition.pop_count < estimation->size()) {
        estimation->resize(estimation->size() - transition.pop_count);
        assert(estimation->back().pattern == transition.pattern);
        if (estimation->back().node_index == transition.node_index) {
            assert(estimation->back().repeat > 0);

            ++estimation->back().repeat;
        } else {
            estimation->back().node_index = transition.node_index;
            estimation->back().repeat = 1;
            estimation->back().repeat_from_start = LoopEstimation::from_start;
        }
    } else {
        if (as_pattern(transition.pattern)->nodes[transition.node_index].pattern ==
            estimation->front().pattern) {
            estimation->clear();
            estimation->emplace_back(EstimationNode { transition.pattern,
                                                      transition.node_index,
                                                      2,
                                                      LoopEstimation::unknown });
        } else {
            estimation->clear();
            estimation->emplace_back(EstimationNode { transition.pattern,
                                                      transition.node_index,
                                                      1,
                                                      LoopEstimation::from_start });
        }
    }
    descend(*estimation);
    return true;
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

    assert(terminal != nullptr);
    assert(terminal->pattern != nullptr);

    if (estimation->size() > 0) {
        for (auto const transition : estimation->back().pattern->transitions) {
            if (transition.terminal == terminal) {
                if (get_estimation_child(estimation, transition)) {
                    assert(as_pattern(estimation->back().pattern)->symbol == terminal);
                    assert(estimation->back().pattern == terminal->pattern);
                    return;
                } else {
                    break;
                }
            }
        }
        estimation->clear();
    }

    estimation->emplace_back(
            EstimationNode { terminal->pattern, 0, 1, LoopEstimation::from_start });

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

        if (get_probability(p) > 0.) {  // TODO precompute this value if possible
            if (p->estimation.size() <= transition.pop_count) {
                return true;
            }

            auto const pattern_index = p->estimation.size() - transition.pop_count - 1;
            auto const & destination = p->estimation[pattern_index];

            if (destination.pattern == transition.pattern) {
                auto const node_index = destination.node_index;
                auto const & node = as_pattern(transition.pattern)->nodes[node_index];
                auto const stay_in_loop = transition.node_index == node_index;
                auto const exit_loop = transition.node_index == node_index + 1;
                assert(node.count >= destination.repeat);
                auto const max_iteration_count_reached = node.count == destination.repeat;

                if (stay_in_loop) {
                    if (max_iteration_count_reached == false) {
                        return true;
                    }
                } else if (exit_loop) {
                    if (destination.repeat_from_start == LoopEstimation::unknown) {
                        return true;
                    } else if (max_iteration_count_reached) {
                        return true;
                    }
                }
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
        get_estimation_child(&p->estimation, transition);
        p->transition_index = 0u;
        return skip_false_prediction(p);
    }
}

static auto get_current_transition(Prediction const * p) -> Transition const * {
    return &p->estimation.back().pattern->transitions[p->transition_index];
}

auto get_terminal(Prediction const * p) -> Terminal const * {
    auto const transition = get_current_transition(p);
    return transition->terminal;
}

static auto compute_transition_occurence_count(Prediction const * p, size_t const index) -> size_t {
    auto const & estimation = p->estimation.back();
    auto const & transition = estimation.pattern->transitions[index];
    if (p->estimation.size() > transition.pop_count) {
        auto const & target = p->estimation[p->estimation.size() - transition.pop_count - 1];
        assert(target.pattern == transition.pattern);
        if (target.node_index == transition.node_index) {
            return transition.ocurence_count - target.repeat + 1;
        }
    }
    return transition.ocurence_count;
}

static auto compute_transition_climb_tree_probability(Prediction const * p) -> double {
    auto count = size_t(0u);
    auto total_count = size_t(0u);

    auto const transition_count = p->estimation.back().pattern->transitions.size();
    for (auto transition_index = 0u; transition_index < transition_count; ++transition_index) {
        auto const transition_count = compute_transition_occurence_count(p, transition_index);
        total_count += transition_count;
        if (transition_index == p->transition_index)
            count = transition_count;
    }

    auto const proba = static_cast<double>(count) / static_cast<double>(total_count);
    assert(proba >= 0. && proba <= 1.);
    return proba;
}

auto get_probability(Prediction const * p) -> double {
    auto const & estimation = p->estimation.back();
    auto const & current_transition = estimation.pattern->transitions[p->transition_index];

    if (p->estimation.size() <= current_transition.pop_count) {
        return compute_transition_climb_tree_probability(p);
    }

    if (p->estimation.size() > current_transition.pop_count + 1) {
        return 1.;
    }

    assert(p->estimation.size() - current_transition.pop_count - 1 == 0);
    auto const & current_target = p->estimation[0];
    assert(current_target.pattern == current_transition.pattern);

    if (current_target.repeat_from_start == LoopEstimation::from_start) {
        auto const node = as_pattern(current_target.pattern)->nodes[current_target.node_index];
        if (current_target.node_index == current_transition.node_index) {
            return node.count >= current_target.repeat ? 1 : 0;
        } else {
            return node.count <= current_target.repeat ? 1 : 0;
        }
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

    auto const proba = static_cast<double>(count) / static_cast<double>(total_count);
    assert(proba >= 0. && proba <= 1.);
    return proba;
}


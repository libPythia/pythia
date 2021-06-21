#pragma once

#include <cassert>

#include "reduction.hpp"

struct Pattern;
struct PatternBase;

struct PatternNode {
    GrammarNode const * node;
    Pattern * pattern;
    size_t count;
};

struct Transition {
    Terminal const * terminal;
    PatternBase * pattern;
    size_t node_index;
    size_t pop_count;
    size_t ocurence_count;
};

struct PatternBase {
    bool is_fake;
    virtual ~PatternBase() = default;
    std::vector<Transition> transitions;

    PatternBase(bool is_fake_) : is_fake(is_fake_) {}
};

struct Pattern : public PatternBase {
    std::vector<PatternNode> nodes;
    size_t size;

    Symbol const * symbol;

    Pattern() : PatternBase(false) {}
};

struct FakePatternOccurence final {
    PatternBase * pattern;
    size_t node_index;
};

struct FakePattern : public PatternBase {
    std::vector<FakePatternOccurence> patterns;

    FakePattern() : PatternBase(true) {}
};

inline auto is_fake_pattern(PatternBase const * p) { return p->is_fake; }
inline auto as_fake_pattern(PatternBase const * p) { return static_cast<FakePattern const *>(p); }
inline auto as_fake_pattern(PatternBase * p) { return static_cast<FakePattern *>(p); }
inline auto as_pattern(PatternBase const * p) { return static_cast<Pattern const *>(p); }
inline auto as_pattern(PatternBase * p) { return static_cast<Pattern *>(p); }

struct FlowGraph {
    std::vector<Pattern> patterns;
    std::vector<Pattern *> roots;
    std::vector<std::unique_ptr<FakePattern>> fake_patterns;
    std::unordered_map<Terminal const *, PatternBase *> terminals_index;
};

auto buildFlowGraph(Grammar & g) -> FlowGraph;

// -----------------------------------------------------------

struct EstimationNode {
    PatternBase const * pattern;
    size_t node_index;
};

using Estimation = std::vector<EstimationNode>;

auto init_estimation(Estimation * e, FlowGraph const * g) -> void;
auto update_estimation(Estimation * e, Terminal const * t) -> void;
auto deinit_estimation(Estimation * e) -> void;

// -----------------------------------------------------------

struct Prediction {
    Estimation estimation;
    size_t transition_index;
};

auto reset_prediction(Prediction * p, Estimation const * e) -> bool;
auto copy_prediction(Prediction * to, Prediction const * from) -> void;
auto get_prediction_tree_child(Prediction * p) -> bool;    // parents = parents->parents puis next
auto get_prediction_tree_sibling(Prediction * p) -> bool;  // parents = parents->next puis next
auto get_terminal(Prediction const * p) -> Terminal const *;
auto get_count(Prediction const * p) -> size_t;
auto get_probability(Prediction const * p) -> double;

// -----------------------------------------------------------


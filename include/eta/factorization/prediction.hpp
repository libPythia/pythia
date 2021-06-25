#pragma once

#include <cassert>

#include "flow_graph.hpp"

// -----------------------------------------------------------

struct EstimationNode {
    PatternBase const * pattern;
    size_t node_index;
    size_t repeat;
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
auto get_probability(Prediction const * p) -> double;

// -----------------------------------------------------------


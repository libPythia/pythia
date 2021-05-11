#pragma once

#include "reduction.hpp"

// -----------------------------------------------------------

struct Estimation {
    struct Parent {
        Node const * node = nullptr;
        Parent * next = nullptr;
        Parent * parents = nullptr;
        size_t count = 0;
    };

    bool at_begining;
    Grammar const * grammar;
    Terminal const * terminal;
    Parent * parents;
    Parent * next_available_parent;
};

auto init_estimation(Estimation * e, Grammar const * g) -> void;
auto update_estimation(Estimation * e, Terminal const * t) -> void;
auto deinit_estimation(Estimation * e) -> void;

// -----------------------------------------------------------

struct Prediction {
    struct Parent {
        Node const * node = nullptr;
        Parent * next = nullptr;
        Parent * parents = nullptr;
        size_t count = 0;
        int ref_count = 0;
    };

    Parent * parents;
    Parent * next_available_parent;
};

auto init_prediction(Prediction * p) -> void;
auto reset_prediction(Prediction * p, Estimation const * e) -> bool;
auto copy_prediction(Prediction * to, Prediction const * from) -> void;
auto get_prediction_tree_child(Prediction * p) -> bool;    // parents = parents->parents puis next
auto get_prediction_tree_sibling(Prediction * p) -> bool;  // parents = parents->next puis next
auto deinit_prediction(Prediction * p) -> void;

// -----------------------------------------------------------


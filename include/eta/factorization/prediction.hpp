#pragma once

#include <cstddef>
#include <vector>

#include "reduction.hpp"

// --------------------------------------------------

using TerminalEqualityOperator = bool (*)(Terminal const *, Terminal const *);
auto terminal_ptr_equality(Terminal const *, Terminal const *) -> bool;

// --------------------------------------------------

struct EvaluationNode {
    GrammarNode const * node;
    unsigned int repeats;
};
using Evaluation = std::vector<EvaluationNode>;
using Estimation = std::vector<Evaluation>;

// --------------------------------------------------

auto init_estimation_from_start(Grammar const & grammar) -> Estimation;
auto next_estimation(Estimation estimation,
                     Terminal const * terminal,
                     TerminalEqualityOperator terminals_are_equivalents = terminal_ptr_equality)
        -> Estimation;

// --------------------------------------------------

struct Prediction {
    Estimation estimation;
    unsigned int index;
};
auto get_prediction_from_estimation(Estimation & e) -> Prediction;
auto get_first_next(Prediction *) -> bool;
auto get_alternative(Prediction *) -> bool;
auto get_terminal(Prediction const &) -> Terminal const *;

// --------------------------------------------------

struct Probability {
    size_t count;
    size_t total;

    auto as_double() const { return static_cast<double>(count) / static_cast<double>(total); };
};

auto get_probability(Estimation const & estimation) -> Probability;

// --------------------------------------------------


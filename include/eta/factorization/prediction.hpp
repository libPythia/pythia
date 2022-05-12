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
    struct info_t {
        size_t index;
        size_t prevalence;
    };
    Estimation estimation;
    std::vector<info_t> infos;
};
auto get_prediction_from_estimation(Estimation const & e) -> Prediction;
auto get_first_next(Prediction *) -> bool;
auto get_alternative(Prediction *) -> bool;
auto get_terminal(Prediction const &) -> Terminal const *;

// --------------------------------------------------

auto get_prevalence(Prediction const & p) -> size_t;

// --------------------------------------------------


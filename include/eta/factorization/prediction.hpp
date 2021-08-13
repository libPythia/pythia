#pragma once

#include "flow_graph.hpp"

enum class RepeatFromStart {
    repeat_from_start,
    repeat_from_unknown_index,
};

class Estimation final {
    friend class Prediction;

    struct EstimationNode final {
        FlowNode const * node;
        size_t repeat;
        RepeatFromStart repeat_from_start;

        EstimationNode(FlowNode const * node_, size_t repeat_, RepeatFromStart repeat_from_start_);
    };

    struct Eventuality final {
        std::vector<EstimationNode> nodes;
    };

  public:
    auto update(FlowGraph const & graph, Terminal const * terminal) -> void;

  private:  // For Prediction
    auto update(FlowGraph const * graph, Terminal const * terminal) -> void;

  private:
    auto updateEventuality(Eventuality const & input,
                           std::vector<Eventuality> & output,
                           Terminal const *) const -> void;
    auto get_parent(Eventuality const & eventuality, size_t pop_count) const
            -> EstimationNode const *;
    auto descend(Eventuality & eventuality) const -> void;
    auto duplicate(Eventuality const & eventuality, size_t pop_count) const -> Eventuality;

  public:  // private: // TODO
    std::vector<Eventuality> eventualities;
};

struct Probability {
    size_t count;
    size_t total;

    auto as_double() const { return static_cast<double>(count) / static_cast<double>(total); };
};

class Prediction {
    // TODO reset and copy

    struct PredictionTransition final {
        Terminal const * terminal;
        size_t count;
    };

  public:
    Prediction() = default;
    Prediction(Prediction &&) = default;
    Prediction(Prediction const &) = default;
    auto operator=(Prediction &&) -> Prediction & = default;
    auto operator=(Prediction const &) -> Prediction & = default;

  public:
    auto reset(Estimation const &) -> bool;
    auto get_prediction_tree_child() -> bool;
    auto get_prediction_tree_sibling() -> bool;
    auto get_probability() const -> Probability;
    auto get_terminal() const -> Terminal const *;

  private:
    auto compute_transitions() -> bool;

  private:
    Estimation estimation;
    std::vector<PredictionTransition> transitions;
    size_t count;
};


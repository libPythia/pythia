#include "prediction.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>

#include "reduction.hpp"

static auto getLogOutput() -> std::ostream & {
    static auto log_output = std::ofstream { "log_output.txt" };
    return log_output;
}

template <typename... T> static auto log(T &&... args) -> void {
    auto & os = getLogOutput();
    ((os << args), ...);
    os << std::endl;
}

static auto operator<<(std::ostream & os, Estimation const & estimation) -> std::ostream & {
    auto first_eventuality = true;
    os << '{';

    for (auto const & eventuality : estimation.eventualities) {
        if (first_eventuality) {
            first_eventuality = false;
        } else {
            os << ", ";
        }

        auto first_node = true;
        for (auto const & node : eventuality.nodes) {
            if (first_node) {
                first_node = false;
                os << "[";
            } else {
                os << ", ";
            }
            os << node.repeat << ":";
            if (node.node->son == nullptr)
                os << (char const *)node.node->first_terminal->payload;
            else
                os << (node.repeat_from_start == RepeatFromStart::repeat_from_start ? "RFS"
                                                                                    : "???");
        }
        if (first_node == false)
            os << "]";
    }
    os << '}';
    return os;
}

// ----------------------------------------------------------------
// EstimationNode
// ----------------------------------------------------------------

Estimation::EstimationNode::EstimationNode(FlowNode const * node_,
                                           size_t repeat_,
                                           RepeatFromStart repeat_from_start_)
        : node(node_)
        , repeat(repeat_)
        , repeat_from_start(repeat_from_start_) {}

// ----------------------------------------------------------------
// Estimation
// ----------------------------------------------------------------

auto Estimation::get_parent(Eventuality const & eventuality, size_t pop_count) const
        -> EstimationNode const * {
    if (pop_count >= eventuality.nodes.size())
        return nullptr;
    auto const index = eventuality.nodes.size() - pop_count - 1;
    return &eventuality.nodes[index];
}

auto Estimation::descend(Eventuality & eventuality) const -> void {
    while (true) {
        auto son = eventuality.nodes.back().node->son;
        if (son == nullptr)
            break;
        eventuality.nodes.emplace_back(son, 1, RepeatFromStart::repeat_from_start);
        son = son->son;
    }
}

auto Estimation::duplicate(Eventuality const & eventuality, size_t pop_count) const -> Eventuality {
    assert(pop_count < eventuality.nodes.size());
    auto res = Eventuality {};
    std::copy(eventuality.nodes.begin(),
              eventuality.nodes.begin() + (eventuality.nodes.size() - pop_count),
              std::back_inserter(res.nodes));
    return res;
}

auto Estimation::updateEventuality(Eventuality const & input,
                                   std::vector<Eventuality> & output,
                                   Terminal const * terminal) const -> void {
    for (auto const & transition : input.nodes.back().node->transitions) {
        auto const terminal_str = (char const *)transition.node->first_terminal->payload;

        if (transition.node->first_terminal == terminal) {
            auto const destination = get_parent(input, transition.pop_count);
            if (destination == nullptr) {  // ascend graph
                auto new_eventuality = Eventuality {};
                auto const loop =
                        input.nodes.size() > 0 && transition.node->son == input.nodes.front().node;
                if (loop) {
                    log("Ascend in loop : ", terminal_str);
                    new_eventuality.nodes.emplace_back(transition.node,
                                                       2,
                                                       RepeatFromStart::repeat_from_unknown_index);
                } else {
                    log("Ascend : ", terminal_str);
                    new_eventuality.nodes.emplace_back(transition.node,
                                                       1,
                                                       RepeatFromStart::repeat_from_start);
                }
                output.emplace_back(std::move(new_eventuality));
            } else if (destination->node == transition.node) {  // loop
                if (destination->repeat < destination->node->repeats) {
                    log("continue loop: ", terminal_str);
                    auto eventuality = duplicate(input, transition.pop_count);
                    eventuality.nodes.back().repeat += 1;
                    output.emplace_back(std::move(eventuality));
                } else {
                    log("ignore, end of loop reached", terminal_str);
                }
            } else if (destination->node->next == transition.node) {  // advance
                if (destination->repeat_from_start == RepeatFromStart::repeat_from_unknown_index ||
                    destination->repeat == destination->node->repeats) {
                    log("reach end of loop: ", terminal_str);
                    auto eventuality = duplicate(input, transition.pop_count);
                    auto & node = eventuality.nodes.back();
                    node.repeat = 1;
                    node.node = transition.node;
                    node.repeat_from_start = RepeatFromStart::repeat_from_start;
                    output.emplace_back(std::move(eventuality));
                } else {
                    log("ignore, loop end not reached : ", terminal_str);
                }
            } else {
                log("transition exit known pattern : ", terminal_str);
            }
        } else {
            log("ignore transition : ", terminal_str);
        }
    }
}

auto Estimation::update(FlowGraph const & graph, Terminal const * terminal) -> void {
    update(&graph, terminal);
}

auto Estimation::update(FlowGraph const * graph, Terminal const * terminal) -> void {
    if (terminal == nullptr) {
        log("Update estimation with unknown terminal");
        eventualities.clear();
    } else {
        log("Update estimation with '", (char const *)terminal->payload);
        auto new_eventualities = std::vector<Eventuality> {};

        for (auto & eventuality : eventualities)
            updateEventuality(eventuality, new_eventualities, terminal);

        if (new_eventualities.size() == 0) {
            log("Restart from nowhere");
            assert(graph != nullptr);
            auto const entry_point = graph->get_entry_point(terminal);
            if (entry_point != nullptr) {
                new_eventualities.emplace_back();
                new_eventualities.back().nodes.emplace_back(
                        entry_point,
                        1,
                        RepeatFromStart::repeat_from_unknown_index);
            }
        } else {
            for (auto & eventuality : new_eventualities)
                descend(eventuality);
        }

        eventualities = std::move(new_eventualities);

        log("Got ", eventualities.size(), " eventualities: ", *this);
    }
}

// ----------------------------------------------------------------
// Prediction
// ----------------------------------------------------------------

auto Prediction::reset(Estimation const & new_estimation) -> bool {
    estimation = new_estimation;
    return compute_transitions();
}

auto Prediction::get_prediction_tree_child() -> bool {
    auto const terminal = get_terminal();
    log("start prediction child");
    estimation.update(nullptr, terminal);
    log("end prediction child");
    return compute_transitions();
}

auto Prediction::get_prediction_tree_sibling() -> bool {
    if (transitions.size() <= 1)
        return false;
    transitions.pop_back();
    return true;
}

auto Prediction::get_probability() const -> Probability {
    return { transitions.back().count, count };
}

auto Prediction::get_terminal() const -> Terminal const * { return transitions.back().terminal; }

auto Prediction::compute_transitions() -> bool {
    transitions.clear();
    count = 0u;

    auto insert_transition = [this](Terminal const * t, size_t c) -> void {
        count += c;
        for (auto & transition : transitions) {
            if (transition.terminal == t) {
                transition.count += c;
                return;
            }
        }
        transitions.emplace_back(PredictionTransition { t, c });
    };

    for (auto const & eventuality : estimation.eventualities) {
        auto const & front = &eventuality.nodes.front();
        for (auto const & transition : eventuality.nodes.back().node->transitions) {
            auto const destination = estimation.get_parent(eventuality, transition.pop_count);

            if (destination == nullptr) {  // ascend
                auto const loop = transition.node->son == front->node;
                insert_transition(
                        transition.node->first_terminal,
                        (loop ? transition.node->repeats - 1 : 1) * transition.node->count);
            } else if (destination->node == transition.node) {
                if (destination->repeat < destination->node->repeats) {
                    if (destination->repeat_from_start == RepeatFromStart::repeat_from_start) {
                        insert_transition(transition.node->first_terminal, front->node->count);
                    } else {
                        insert_transition(transition.node->first_terminal,
                                          front->node->count * (destination->node->repeats -
                                                                destination->repeat));
                    }
                }
            } else if (destination->node->next == transition.node) {  // advance
                if (destination->repeat_from_start == RepeatFromStart::repeat_from_unknown_index ||
                    destination->repeat == destination->node->repeats) {
                    insert_transition(transition.node->first_terminal,
                                      front->node->count);  // TODO
                }
            }
        }
    }

    std::sort(transitions.begin(),
              transitions.end(),
              [](auto const & lhs, auto const & rhs) -> bool { return lhs.count < rhs.count; });

    return transitions.size() > 0;
}

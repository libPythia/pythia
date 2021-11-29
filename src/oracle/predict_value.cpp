#include "predict_value.h"

#include <cassert>
#include <eta/factorization/export.hpp>
#include <eta/factorization/prediction.hpp>
#include <eta/factorization/reduction.hpp>
#include <iostream>

// ---------------------------------------------------

enum class Mode { Recording, Predicting };
static auto mode = Mode::Recording;

static constexpr auto values_count = 256u;  // unsigned char

// ---------------------------------------------------

struct Payload {
    enum Type { Predict, Correct, Custom };

    Terminal const * terminal;
    int id;
    Type type;
};

struct EvaluationNode {
    GrammarNode const * node;
};
using Evaluation = std::vector<EvaluationNode>;

static Grammar grammar;
static NonTerminal * nonterminal = nullptr;
static NonTerminal * root = nullptr;

static std::vector<Payload> payloads;
static Terminal * ask_prediction = nullptr;
static std::vector<Terminal *> predictions;
static std::vector<Terminal *> events;

static std::vector<Evaluation> evaluations;

// --------------------------------------------------

extern "C" {

void eta_deinit_value_oracle() {
    grammar = Grammar {};
    nonterminal = nullptr;
    root = nullptr;
    payloads.clear();
    ask_prediction = nullptr;
    predictions.clear();
    events.clear();
    evaluations.clear();
}
}

// ---------------------------------------------------

static auto get_payload(Evaluation const & e) -> Payload const * {
    auto const terminal = as_terminal(e.back().node->maps_to);
    return static_cast<Payload const *>(terminal->payload);
}

static auto are_equivalent(Payload const * lhs, Payload const * rhs) -> bool {
    if (lhs->type != rhs->type)
        return false;
    if (lhs->type == Payload::Type::Custom)
        return lhs->id == rhs->id;
    return true;
}

static auto descend_evaluation(Evaluation & eval) -> void {
    while (true) {
        auto const symbol = eval.back().node->maps_to;
        if (is_terminal(symbol))
            break;
        eval.push_back({ as_nonterminal(symbol)->first });
    }
}

template <typename F> static auto next_evaluation(Evaluation eval, F && add_evaluation) {
    while (!eval.empty()) {
        auto const next_object = eval.back().node->next;
        if (is_node(next_object)) {
            eval.back().node = as_node(next_object);
            descend_evaluation(eval);

            add_evaluation(std::move(eval));
            return;
        } else {
            eval.pop_back();
        }
    }

    // TODO remonter les parents
}

static auto next_evaluations(std::vector<Evaluation> evals, Payload new_payload)
        -> std::vector<Evaluation> {
    auto res = std::vector<Evaluation> {};
    for (auto & eval : evals) {
        auto const payload = get_payload(eval);
        if (are_equivalent(&new_payload, payload)) {
            next_evaluation(std::move(eval), [&res](auto e) { res.push_back(std::move(e)); });
        }
    }
    return res;
}

// ---------------------------------------------------

extern "C" {

// ---------------------------------------------------

void eta_init_value_oracle_recording(unsigned int event_type_count) {
    mode = Mode::Recording;
    // terminal payload meaning
    // [-256, -2] => predictions
    // -1 => ask for prediction
    // [0,inf) => custom events

    payloads.reserve(values_count + event_type_count + 1);

    // Predict event
    payloads.emplace_back();
    ask_prediction = new_terminal(grammar, &payloads.back());
    payloads.back().type = Payload::Type::Predict;
    payloads.back().terminal = ask_prediction;

    // Correct prediction events
    predictions.reserve(values_count);
    for (auto i = 0u; i < values_count; ++i) {
        payloads.emplace_back();
        predictions.push_back(new_terminal(grammar, &payloads.back()));
        payloads.back().type = Payload::Type::Correct;
        payloads.back().id = i;
        payloads.back().terminal = predictions.back();
    }

    // Custom user event
    events.reserve(event_type_count);
    for (auto i = 0u; i < event_type_count; ++i) {
        payloads.emplace_back();
        events.push_back(new_terminal(grammar, &payloads.back()));
        payloads.back().type = Payload::Type::Custom;
        payloads.back().id = i;
        payloads.back().terminal = events.back();
    }
};

// ---------------------------------------------------

void eta_switch_value_oracle_to_prediction() {
    assert(mode == Mode::Recording);
    mode = Mode::Predicting;

    evaluations.emplace_back();
    evaluations.back().push_back({ root->first });
    descend_evaluation(evaluations.back());

    print_reduced_trace(root, std::cout, [](Terminal const * t, std::ostream & os) {
        auto const payload = reinterpret_cast<Payload const *>(t->payload);
        switch (payload->type) {
            case Payload::Type::Predict: {
                os << "<predict>";
            } break;
            case Payload::Type::Correct: {
                os << "<correct=" << payload->id << '>';
            } break;
            case Payload::Type::Custom: {
                os << '[' << payload->id << ']';
            } break;
        }
    });
    std::cout << std::endl;
}

// ---------------------------------------------------

unsigned char eta_predict_value(unsigned char default_value) {
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(grammar, nonterminal, ask_prediction);
            if (root == nullptr)
                root = nonterminal;
            return default_value;
        }
        case Mode::Predicting: {
            evaluations = next_evaluations(std::move(evaluations),
                                           Payload { nullptr, 0, Payload::Type::Predict });

            // TODO search most probable futur prediction correction
            auto const payload = get_payload(evaluations[0]);  // TODO
            assert(payload->type == Payload::Type::Correct);   // TODO
            auto const value = payload->id;                    // TODO predict
            std::cout << "<predict=" << (int)value << '>';     // TODO
            return value;
        };
    }
    assert(false);
}

// ---------------------------------------------------

void eta_correct_value_prediction(unsigned char value) {
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(grammar, nonterminal, predictions[value]);
        } break;
        case Mode::Predicting: {
            evaluations = next_evaluations(std::move(evaluations),
                                           Payload { nullptr, 0, Payload::Type::Correct });
            std::cout << "<correct>";
        } break;
    }
}

// ---------------------------------------------------

void eta_append_event(unsigned int event_type) {
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(grammar, nonterminal, events[event_type]);
        } break;
        case Mode::Predicting: {
            std::cout << '[' << event_type << ']';  // TODO
            evaluations = next_evaluations(
                    std::move(evaluations),
                    Payload { nullptr, static_cast<int>(event_type), Payload::Type::Custom });
        } break;
    }
}

// ---------------------------------------------------

}  // extern "C"

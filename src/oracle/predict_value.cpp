#include "predict_value.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    unsigned int repeats;
};
using Evaluation = std::vector<EvaluationNode>;

static Grammar grammar;
static NonTerminal * nonterminal = nullptr;
static NonTerminal * root = nullptr;  // TODO Remove

static Payload * payloads;
static Terminal * ask_prediction = nullptr;
static Terminal ** predictions;
static Terminal ** events;

static std::vector<Evaluation> evaluations;
static char const * trace_path = nullptr;

// --------------------------------------------------

extern "C" {

void eta_deinit_value_oracle() {
    grammar = Grammar {};
    nonterminal = nullptr;
    root = nullptr;
    free(payloads);
    ask_prediction = nullptr;
    free(predictions);
    free(events);
    evaluations.clear();
}
}

// ---------------------------------------------------

static auto get_payload(Evaluation const & e) -> Payload const * {
    auto const terminal = as_terminal(e.back().node->maps_to);
    return static_cast<Payload const *>(terminal->payload);
}

static auto count_possible_occurrences(Evaluation const & e) -> size_t {
    auto aux = [](auto rec, GrammarNode const * n) -> size_t {
        auto symbol = [n]() {
            auto s = n->next;
            while (is_node(s))
                s = as_node(s)->next;
            return as_symbol(s);
        }();
        auto res = size_t(0);
        for (auto const & parent : symbol->occurrences_without_successor) {
            res += parent->repeats * rec(rec, parent);
        }
        for (auto const & [_, parent] : symbol->occurrences_with_successor) {
            res += parent->repeats * rec(rec, parent);
        }
        return res == 0 ? 1 : res;
    };
    return (e.front().node->repeats - e.front().repeats) * aux(aux, e.front().node);
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
        eval.push_back({ as_nonterminal(symbol)->first, 0 });
    }
}

template <typename F>
static auto next_evaluation_ascend(Symbol const * symbol, F && add_evaluation) -> void {
    auto aux = [&add_evaluation](GrammarNode const * node) {
        if (is_node(node->next)) {
            auto eval = Evaluation { EvaluationNode { as_node(node->next), 0 } };
            descend_evaluation(eval);
            add_evaluation(std::move(eval));
        } else {
            next_evaluation_ascend(as_symbol(node->next), add_evaluation);
        }
    };
    for (auto const & parent : symbol->occurrences_without_successor)
        aux(parent);
    for (auto const & [_, parent] : symbol->occurrences_with_successor)
        aux(parent);
}

template <typename F> static auto next_evaluation(Evaluation eval, F && add_evaluation) -> void {
    while (true) {
        auto & eval_node = eval.back();
        auto const next_object = eval_node.node->next;
        if (++eval_node.repeats < eval_node.node->repeats) {  // in loop
            descend_evaluation(eval);
            add_evaluation(std::move(eval));
            break;
        } else if (is_node(next_object)) {  // end loop but has trivial successor
            eval.back().node = as_node(next_object);
            eval.back().repeats = 0;
            descend_evaluation(eval);

            add_evaluation(std::move(eval));
            break;
        } else if (eval.size() == 1) {  // top of eval reached : gain knowledge by exploring parents
            next_evaluation_ascend(as_symbol(next_object), add_evaluation);
            break;
        } else {
            eval.pop_back();  // use existing knowledge in order to find non trivial next
        }
    }
}

static auto next_evaluations(std::vector<Evaluation> evals, Terminal const * terminal)
        -> std::vector<Evaluation> {
    auto res = std::vector<Evaluation> {};
    bool loose_knowledge = true;
    for (auto & eval : evals) {
        auto const payload = get_payload(eval);
        if (are_equivalent(static_cast<Payload const *>(terminal->payload), payload)) {
            loose_knowledge = false;
            next_evaluation(std::move(eval), [&res](auto e) { res.push_back(std::move(e)); });
        }
    }

    if (loose_knowledge) {
        auto eval = Evaluation {};
        for (auto const parent : terminal->occurrences_without_successor) {
            eval.push_back(EvaluationNode { parent, 1 });
            next_evaluation(std::move(eval), [&res](auto e) { res.push_back(std::move(e)); });
        }
        for (auto const & [_, parent] : terminal->occurrences_with_successor) {
            eval.push_back(EvaluationNode { parent, 1 });
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

    payloads = static_cast<decltype(payloads)>(
            malloc((values_count + event_type_count + 1) * sizeof(payloads[0])));
    auto next_payload = 0u;

    // Predict event
    // payloads.emplace_back();
    ask_prediction = new_terminal(grammar, &payloads[0]);
    payloads[0].type = Payload::Type::Predict;
    payloads[0].terminal = ask_prediction;
    ++next_payload;

    // Correct prediction events
    predictions = static_cast<decltype(predictions)>(malloc(values_count * sizeof(predictions[0])));
    for (auto i = 0u; i < values_count; ++i) {
        predictions[i] = new_terminal(grammar, &payloads[next_payload]);
        payloads[next_payload].type = Payload::Type::Correct;
        payloads[next_payload].id = i;
        payloads[next_payload].terminal = predictions[i];
        ++next_payload;
    }

    // Custom user event
    events = static_cast<decltype(events)>(malloc(event_type_count * sizeof(events[0])));
    for (auto i = 0u; i < event_type_count; ++i) {
        events[i] = new_terminal(grammar, &payloads[next_payload]);
        payloads[next_payload].type = Payload::Type::Custom;
        payloads[next_payload].id = i;
        payloads[next_payload].terminal = events[i];
        ++next_payload;
    }
};

void eta_init_value_oracle(unsigned int event_type_count) {
    auto const path = []() {
        auto p = getenv("ETA_TRACE");
        return (p == nullptr || strcmp(p, "") == 0) ? "trace.btr" : p;
    }();

    if (access(path, F_OK)) {
        mode = Mode::Predicting;
        // TODO
    } else {
        eta_init_value_oracle_recording(event_type_count);
    }
}

// ---------------------------------------------------

void eta_switch_value_oracle_to_prediction() {
    assert(mode == Mode::Recording);
    mode = Mode::Predicting;

    print_grammar(grammar, std::cout, [](Terminal const * t, std::ostream & os) {
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

    evaluations.emplace_back();
    assert(nonterminal == root);
    evaluations.back().push_back({ root->first, 0 });
    descend_evaluation(evaluations.back());
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
            evaluations = next_evaluations(std::move(evaluations), ask_prediction);

            auto const value = [&]() -> unsigned char {
                switch (evaluations.size()) {
                    case 0: return default_value;
                    case 1: {
                        auto const & eval = evaluations[0];
                        assert(get_payload(eval)->type == Payload::Type::Correct);
                        return get_payload(eval)->id;
                    }
                    default: {
                        auto res = default_value;
                        auto buf = std::array<size_t, values_count> {};
                        for (auto & i : buf)
                            i = 0;

                        for (auto const & eval : evaluations) {
                            assert(get_payload(eval)->type == Payload::Type::Correct);
                            auto const v = get_payload(eval)->id;
                            buf[v] += count_possible_occurrences(eval);
                            // printf("{%d->%d}", v, buf[v]);
                            if (buf[res] < buf[v])
                                res = v;
                        }

                        return res;
                    }
                }
            }();
            return value;
        };
    }
    assert(false);
}

// ---------------------------------------------------

void eta_correct_value_prediction(unsigned char value) {
    auto const terminal = predictions[value];
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(grammar, nonterminal, terminal);
            if (root == nullptr)
                root = nonterminal;
        } break;
        case Mode::Predicting: {
            evaluations = next_evaluations(std::move(evaluations), terminal);
        } break;
    }
}

// ---------------------------------------------------

void eta_append_event(unsigned int event_type) {
    auto const terminal = events[event_type];
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(grammar, nonterminal, terminal);
            if (root == nullptr)
                root = nonterminal;
        } break;
        case Mode::Predicting: {
            evaluations = next_evaluations(std::move(evaluations), terminal);
        } break;
    }
}

// ---------------------------------------------------

}  // extern "C"

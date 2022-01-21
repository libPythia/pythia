#include "predict_value.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/prediction.hpp>
#include <eta/factorization/reduction.hpp>
#include <fstream>
#include <iostream>

// ---------------------------------------------------

enum class Mode { Recording, Predicting };
static auto mode = Mode::Recording;

static constexpr auto values_count = 256u;
using value_type = unsigned char;

// ---------------------------------------------------

struct Payload {
    enum Type { Predict, Correct, Custom };

    Terminal * terminal;
    unsigned int id;
    Type type;
};

struct EvaluationNode {
    GrammarNode const * node;
    unsigned int repeats;
};
using Evaluation = std::vector<EvaluationNode>;

static Grammar * grammar;
static NonTerminal * nonterminal = nullptr;
static NonTerminal const * root = nullptr;

namespace Payloads {
static Payload ask_prediction;
static Payload * predictions;
static Payload * events;
}  // namespace Payloads

static std::vector<Evaluation> evaluations;
static char const * trace_path = nullptr;

// --------------------------------------------------

static auto get_payload(Terminal const * terminal) -> Payload const * {
    return static_cast<Payload const *>(terminal->payload);
}

static auto get_payload(Evaluation const & e) -> Payload const * {
    return get_payload(as_terminal(e.back().node->maps_to));
}

// --------------------------------------------------

static auto get_correction(value_type value) -> Payload * {
    using namespace Payloads;
    assert(predictions != nullptr);
    auto const payload = &predictions[value];
    assert(get_payload(payload->terminal) == payload);
    assert(payload->type == Payload::Type::Correct);
    assert(payload->id == value);
    return payload;
}

static auto get_prediction() -> Payload * {
    using namespace Payloads;
    assert(get_payload(ask_prediction.terminal) == &ask_prediction);
    assert(ask_prediction.type == Payload::Type::Predict);
    return &ask_prediction;
}

static auto get_event(unsigned int id) -> Payload * {
    using namespace Payloads;
    assert(events != nullptr);
    auto const payload = &events[id];
    assert(get_payload(payload->terminal) == payload);
    assert(payload->type == Payload::Type::Custom);
    assert(payload->id == id);
    return payload;
}

static auto init(unsigned int event_type_count) -> void {
    using namespace Payloads;

    grammar = new Grammar {};

    // Predict event
    ask_prediction.type = Payload::Type::Predict;

    // Correct prediction events
    predictions = static_cast<decltype(predictions)>(malloc(values_count * sizeof(predictions[0])));
    for (auto i = 0u; i < values_count; ++i) {
        predictions[i].type = Payload::Type::Correct;
        predictions[i].id = i;
    }

    // Custom user event
    events = static_cast<decltype(events)>(malloc(event_type_count * sizeof(events[0])));
    for (auto i = 0u; i < event_type_count; ++i) {
        events[i].type = Payload::Type::Custom;
        events[i].id = i;
    }

    trace_path = []() {
        auto p = getenv("ETA_TRACE");
        return (p == nullptr || strcmp(p, "") == 0) ? "trace.btr" : p;
    }();
}

static auto bind_payload_to_terminal(Payload * p, Terminal * t) -> void {
    p->terminal = t;
    t->payload = p;
}

static auto init_evaluation_from_start() -> void;

static auto eta_init_value_oracle_predicting(unsigned int event_type_count) {
    mode = Mode::Predicting;
    init(event_type_count);

    auto file = std::ifstream { trace_path };
    load_bin_file(*grammar, file);
    for (auto & terminal : grammar->terminals) {
        auto str = static_cast<char const *>(terminal->payload);
        unsigned int id;
        char type;
        sscanf(str, "%c%u", &type, &id);
        delete str;
        auto payload = [&]() -> Payload * {
            switch (type) {
                case 'P': return &Payloads::ask_prediction;
                case 'C': {
                    assert(id < values_count);
                    return &Payloads::predictions[id];
                }
                case 'E': {
                    assert(id < event_type_count);
                    return &Payloads::events[id];
                }
            }
            assert(false);  // unreachable
        }();
        assert(payload->id == id);
        bind_payload_to_terminal(payload, terminal.get());
    }

    for (auto const nonterminal : grammar->nonterminals.in_use_nonterminals()) {
        if (occurrences_count(nonterminal) == 0) {
            root = nonterminal;
            break;
        }
    }

    init_evaluation_from_start();
}

// --------------------------------------------------

extern "C" {

int eta_is_prediction_enabled() { return static_cast<int>(mode == Mode::Predicting); }

void eta_init_value_oracle_recording(unsigned int event_type_count) {
    mode = Mode::Recording;
    init(event_type_count);

    bind_payload_to_terminal(&Payloads::ask_prediction, new_terminal(*grammar, nullptr));

    for (auto i = 0u; i < event_type_count; ++i)
        bind_payload_to_terminal(&Payloads::events[i], new_terminal(*grammar, nullptr));

    for (auto i = 0u; i < values_count; ++i)
        bind_payload_to_terminal(&Payloads::predictions[i], new_terminal(*grammar, nullptr));
};

void eta_init_value_oracle(unsigned int event_type_count) {
    switch ([]() {
        auto v = getenv("ETA_MODE");
        return (v != nullptr && strcmp(v, "PREDICT") == 0) ? Mode::Predicting : Mode::Recording;
    }()) {
        case Mode::Predicting: eta_init_value_oracle_predicting(event_type_count); break;
        case Mode::Recording: eta_init_value_oracle_recording(event_type_count); break;
    }
}

// ---------------------------------------------------

void eta_deinit_value_oracle() {
    if (mode == Mode::Recording) {
        auto file = std::ofstream { trace_path };
        print_bin_file(*grammar, file, [](Terminal const * t, std::ostream & os) -> void {
            auto const payload = get_payload(t);
            switch (payload->type) {
                case Payload::Type::Correct: os << 'C' << payload->id; break;
                case Payload::Type::Custom: os << 'E' << payload->id; break;
                case Payload::Type::Predict: os << 'P' << 0; break;
            }
        });
    }
    delete grammar;
    nonterminal = nullptr;
    root = nullptr;

    free(Payloads::predictions);
    free(Payloads::events);

    evaluations.clear();
}

// ---------------------------------------------------

}  // extern "C"

// ---------------------------------------------------

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

static auto init_evaluation_from_start() -> void {
    evaluations.emplace_back();
    // assert(nonterminal == root);
    evaluations.back().push_back({ root->first, 0 });
    descend_evaluation(evaluations.back());
}

// ---------------------------------------------------

extern "C" {

// ---------------------------------------------------

void eta_switch_value_oracle_to_prediction() {
    assert(mode == Mode::Recording);
    mode = Mode::Predicting;
    init_evaluation_from_start();
}

// ---------------------------------------------------

unsigned char eta_predict_value(unsigned char default_value) {
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(*grammar, nonterminal, get_prediction()->terminal);
            if (root == nullptr)
                root = nonterminal;
            return default_value;
        }
        case Mode::Predicting: {
            evaluations = next_evaluations(std::move(evaluations), get_prediction()->terminal);

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
    auto const terminal = get_correction(value)->terminal;
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(*grammar, nonterminal, terminal);
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
    auto const terminal = get_event(event_type)->terminal;
    switch (mode) {
        case Mode::Recording: {
            nonterminal = insertSymbol(*grammar, nonterminal, terminal);
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

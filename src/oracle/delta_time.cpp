#include "delta_time.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/prediction.hpp>
#include <eta/factorization/reduction.hpp>
#include <fstream>
#include <string>
#include <unordered_map>

// ---------------------------------------------------

template <typename T> void hash_combine(std::size_t & seed, T const & v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <> struct std::hash<Evaluation> {
    std::size_t operator()(Evaluation const & e) const noexcept {
        size_t h = 0;
        for (auto const & n : e) {
            hash_combine<void const *>(h, n.node);
            // hash_combine(h, n.repeats);  // TODO
        }
        return h;
    }
};

template <> struct std::hash<std::pair<size_t, size_t>> {
    std::size_t operator()(std::pair<size_t, size_t> const & p) const noexcept {
        size_t h = 0;
        hash_combine<size_t>(h, p.first);
        hash_combine<size_t>(h, p.second);
        return h;
    }
};

// ---------------------------------------------------

enum class Mode {
    Disabled,
    Predicting,
    Recording,
    PredictingForTest,
};
static Mode mode;

// ---------------------------------------------------

auto operator==(EvaluationNode const & lhs, EvaluationNode const & rhs) -> bool {
    return lhs.node == rhs.node;  // TODO && lhs.repeats == rhs.repeats;
}

// ---------------------------------------------------

using Timestamp = unsigned long long int;
using Timestamps = std::vector<Timestamp>;

using DeltaTime = struct { size_t total, count; };
using DeltaTimes = std::unordered_map<Evaluation, DeltaTime>;

static auto get_timestamp() -> Timestamp {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

// ---------------------------------------------------

struct RecordingData {
    Grammar grammar;
    NonTerminal * root = nullptr;
    Timestamps timestamps;
};
static RecordingData * recording_data = nullptr;

struct PredictingData {
    Grammar grammar;
    Estimation estimation;
    DeltaTimes delta_times;
};
static PredictingData * predicting_data = nullptr;

// ---------------------------------------------------

static auto get_terminal(Grammar const & grammar, unsigned int id) {
    assert(id < grammar.terminals.size());
    return grammar.terminals[id].get();
}

// ---------------------------------------------------

static auto add_all_subpath(DeltaTimes & dts, Evaluation eval, size_t dt) -> void {
    while (eval.size() > 0) {
        auto & entry = dts[eval];
        ++entry.count;
        entry.total += dt;
        eval.pop_back();
    }
}

static auto compute_deltas(Grammar const & grammar, Timestamps timestamps) -> DeltaTimes {
    auto delta_times = DeltaTimes {};
    auto timestamp_index = size_t(0);

    assert(timestamps.size() > 0);
    auto last = timestamps[0];

    auto estimation = init_estimation_from_start(grammar);
    while (estimation.size() > 0) {
        assert(estimation.size() == 1);

        auto ts = timestamps[timestamp_index];
        add_all_subpath(delta_times, estimation[0], ts - last);
        last = ts;
        ++timestamp_index;

        estimation = next_estimation(estimation, as_terminal(estimation[0].back().node->maps_to));
    }

    return delta_times;
}

// ---------------------------------------------------

static auto get_trace_path() -> std::pair<std::string, std::string> {
    auto const p = getenv("ETA_TRACE");
    auto path = std::string { (p == nullptr || strcpy(p, "") == 0) ? "trace" : p };
    return { path + "_grammar.btr", path + "_dts.bin" };
}

template <typename Insert> static auto build_node_index(Grammar & grammar, Insert && insert) {
    auto nonterminals = grammar.nonterminals.in_use_nonterminals();
    for (auto nonterminal_index = 0u; nonterminal_index < nonterminals.size();
         ++nonterminal_index) {
        auto node = nonterminals[nonterminal_index]->first;
        for (auto node_index = 0u; true; ++node_index) {
            insert(node, nonterminal_index, node_index);
            if (is_nonterminal(node->next))
                break;
            node = as_node(node->next);
        }
    }
}

// dts binary file format all on 8 bytes
// path_count
// path_size node_indices... cumulated_duration_ns call_count
// ...

static auto export_trace() -> void {
    assert(mode == Mode::Recording);
    assert(recording_data != nullptr);
    auto const [grammar_path, dts_path] = get_trace_path();

    {
        auto grammar_file = std::ofstream { grammar_path };
        print_bin_file(recording_data->grammar,
                       grammar_file,
                       [](Terminal const * t, std::ostream & os) -> void {
                           os << reinterpret_cast<uintptr_t>(t->payload);
                       });
    }

    auto node_table = std::unordered_map<GrammarNode const *, std::pair<size_t, size_t>> {};
    build_node_index(recording_data->grammar,
                     [&](GrammarNode const * node, size_t nonterminal_index, size_t node_index) {
                         node_table[node] = { nonterminal_index, node_index };
                     });

    auto dts_file = std::ofstream { dts_path, std::ofstream::binary };
    auto const dts = compute_deltas(recording_data->grammar, recording_data->timestamps);

    auto write = [&dts_file](size_t arg) {
        dts_file.write(reinterpret_cast<char const *>(&arg), sizeof(arg));
    };

    write(dts.size());
    for (auto const & [path, stats] : dts) {
        write(path.size());
        for (auto const & step : path) {
            auto const & node_indices = node_table[step.node];
            write(node_indices.first);
            write(node_indices.second);
            // step.repeats // TODO
        }
        write(stats.total);
        write(stats.count);
    }
}

static auto import_trace() -> void {
    assert(mode == Mode::Predicting);
    assert(predicting_data != nullptr);
    auto const [grammar_path, dts_path] = get_trace_path();

    {
        auto grammar_file = std::ifstream { grammar_path };
        load_bin_file(predicting_data->grammar, grammar_file);
    }

    for (auto terminal_index = 0u; terminal_index < predicting_data->grammar.terminals.size();
         ++terminal_index) {
        auto const terminal = predicting_data->grammar.terminals[terminal_index].get();
        auto payload_index = 0u;
        sscanf(static_cast<char const *>(terminal->payload), "%u", &payload_index);
        assert(payload_index == terminal_index);
        std::free(terminal->payload);
        terminal->payload = reinterpret_cast<void *>(terminal_index);
    }

    auto node_table = std::unordered_map<std::pair<size_t, size_t>, GrammarNode const *> {};
    build_node_index(predicting_data->grammar,
                     [&](GrammarNode const * node, size_t nonterminal_index, size_t node_index) {
                         node_table[std::pair { nonterminal_index, node_index }] = node;
                     });

    auto dts_file = std::ifstream { dts_path, std::ifstream::binary };
    auto read = [&dts_file]() {
        auto buf = size_t(0);
        dts_file.read(reinterpret_cast<char *>(&buf), sizeof(buf));
        return buf;
    };

    auto const path_count = read();
    predicting_data->delta_times.reserve(path_count);

    for (auto path_index = 0u; path_index < path_count; ++path_index) {
        auto const path_size = read();
        auto eval = Evaluation {};
        for (auto step_index = 0u; step_index < path_size; ++step_index) {
            auto const nonterminal_index = read();
            auto const node_index = read();
            eval.push_back(EvaluationNode { node_table[std::pair { nonterminal_index, node_index }],
                                            0u });  // TODO
        }
        auto & stats = predicting_data->delta_times[eval];
        stats.total = read();
        stats.count = read();
    }
}

// ---------------------------------------------------

extern "C" {

// ---------------------------------------------------

int eta_dt_oracle_is_active() { return mode != Mode::Disabled; }

int eta_dt_oracle_is_prediction_enabled() {
    switch (mode) {
        case Mode::Disabled: return false;
        case Mode::Predicting: return true;
        case Mode::PredictingForTest: return true;
        case Mode::Recording: return false;
    }
    assert(false);
}

void eta_dt_oracle_init(unsigned int event_type_count) {
    assert(predicting_data == nullptr);
    assert(recording_data == nullptr);

    switch ([]() {
        auto v = getenv("ETA_MODE");
        if (v != nullptr) {
            if (strcmp(v, "PREDICT") == 0)
                return Mode::Predicting;
            else if (strcmp(v, "RECORD") == 0)
                return Mode::Recording;
        }
        return Mode::Disabled;
    }()) {
        case Mode::Disabled: {
            mode = Mode::Disabled;
            predicting_data = nullptr;
            recording_data = nullptr;
        } break;
        case Mode::Predicting: {
            mode = Mode::Predicting;
            predicting_data = new PredictingData {};
            recording_data = nullptr;
            import_trace();
            predicting_data->estimation = init_estimation_from_start(predicting_data->grammar);
        } break;
        case Mode::Recording: {
            mode = Mode::Recording;
            recording_data = new RecordingData {};
            predicting_data = nullptr;
            while (recording_data->grammar.terminals.size() < event_type_count)
                new_terminal(recording_data->grammar,
                             (void *)uintptr_t(recording_data->grammar.terminals.size()));
        } break;
        case Mode::PredictingForTest: assert(false); break;
    }
}

void eta_dt_oracle_switch_to_prediction() {
    assert(predicting_data == nullptr);
    assert(recording_data != nullptr);
    assert(mode == Mode::Recording);

    mode = Mode::PredictingForTest;

    predicting_data = new PredictingData {};

    predicting_data->grammar = std::move(recording_data->grammar);
    predicting_data->delta_times =
            compute_deltas(predicting_data->grammar, recording_data->timestamps);
    predicting_data->estimation = init_estimation_from_start(predicting_data->grammar);

    delete recording_data;
    recording_data = nullptr;
}

void eta_dt_oracle_deinit() {
    if (mode == Mode::Recording) {
        export_trace();
    }
}

void eta_dt_oracle_add_event(unsigned int event_id) {
    switch (mode) {
        case Mode::Disabled: break;
        case Mode::Recording: {
            assert(recording_data != nullptr);
            recording_data->root = insertSymbol(recording_data->grammar,
                                                recording_data->root,
                                                get_terminal(recording_data->grammar, event_id));
            recording_data->timestamps.push_back(get_timestamp());
        } break;
        case Mode::Predicting:
        case Mode::PredictingForTest: {
            assert(predicting_data != nullptr);
            predicting_data->estimation =
                    next_estimation(std::move(predicting_data->estimation),
                                    get_terminal(predicting_data->grammar, event_id));
        } break;
    }
}

void eta_dt_oracle_get_prediction(eta_dt_oracle_prediction * prediction) {
    assert(predicting_data != nullptr);
    auto const & measure = predicting_data->delta_times[predicting_data->estimation[0]];
    auto const dt_ns = static_cast<double>(measure.total) / static_cast<double>(measure.count);
    prediction->dt = dt_ns * 1e-9;
    prediction->type =
            (uintptr_t)as_terminal(predicting_data->estimation[0].back().node->maps_to)->payload;
}

// ---------------------------------------------------

}  // extern "C"

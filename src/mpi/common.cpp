#include "common.hpp"

#include <mpi.h>
#include <stdlib.h>

#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/reduction.hpp>
#include <fstream>
#include <iostream>
#include <tuple>
#include <unordered_map>

struct pair_hash {
    template <class T1, class T2> std::size_t operator()(const std::pair<T1, T2> & pair) const {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};

struct Payload {
    Pythia_MPI_fn fn;
    int arg1;
    int arg2;
};

struct Data {
    struct Event {
        std::unordered_map<std::pair<int, int>, Terminal *, pair_hash> terminals;
    };
    std::array<Event, pythia_fn_count> events;

    std::atomic_int recursion_count = 0;
    bool is_recording = true;
    bool log = false;

    Grammar grammar;
    NonTerminal * root = nullptr;

    char file_name[1024];
};
static Data * data;

// -------------------------------------------------------------

auto pythia_event(Pythia_MPI_fn fn, int arg1, int arg2) -> void {
    if (++data->recursion_count == 1) {
        if (data->log)
            fprintf(stderr, "Pythia raised event %s with arg %d\n", pythia_MPI_fn_name(fn), arg1);

        auto const terminal = [&]() -> Terminal * {
            auto const [it, inserted] =
                    data->events[(int)fn].terminals.try_emplace({ arg1, arg2 }, nullptr);
            if (inserted && data->is_recording) {
                it->second = new_terminal(data->grammar, new Payload { fn, arg1 });
            }
            return it->second;
        }();

        if (data->is_recording) {
            data->root = insertSymbol(data->grammar, data->root, terminal);
        } else {
            assert(false);
        }
    }
    --data->recursion_count;
}

// -------------------------------------------------------------

static auto serialize(Terminal const * t, std::ostream & os) -> void {
    auto const p = (Payload const *)t->payload;
    os << (int)p->fn << ':' << p->arg1;
}

static auto deserialize(std::istream & is, size_t size) -> void * {
    char buf[2048];
    assert(size < 2048);
    assert(size > 0);
    is.read(buf, size);
    buf[size] = 0;
    int fn, arg1;
    sscanf(buf, "%d:%d", &fn, &arg1);
    return new Payload { (Pythia_MPI_fn)fn, arg1 };
}

auto pythia_init() -> void {
    data = new Data {};

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    sprintf(data->file_name, "pythia_MPI_%d.btr", world_rank);

    auto const log_env = getenv("PYTHIA_MPI_LOG");
    data->log = (log_env != nullptr && strcmp(log_env, "YES") == 0);

    auto const mode_env = getenv("PYTHIA_MPI");
    data->is_recording = (mode_env != nullptr && strcmp(mode_env, "Predict") == 0) == false;
    if (data->log)
        fprintf(stderr, data->is_recording ? "Pythia MPI recording\n" : "Pythia MPI predicting\n");

    if (data->is_recording == false) {
        auto file = std::ifstream { data->file_name };
        if (data->log)
            fprintf(stderr, "Load trace from file %s\n", data->file_name);
        load_bin_file(data->grammar, file, deserialize);

        for (auto & terminal : data->grammar.terminals) {
            auto const p = (Payload const *)terminal->payload;
            data->events[(int)p->fn].terminals[std::pair { p->arg1, p->arg2 }] = terminal.get();
        }
    }
}

auto pythia_deinit() -> void {
    if (data->is_recording) {
        if (data->log)
            fprintf(stderr, "Export trace in file %s\n", data->file_name);
        auto file = std::ofstream { data->file_name };
        print_bin_file(data->grammar, file, serialize);
    }
    delete data;
}

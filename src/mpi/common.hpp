#pragma once

auto pythia_init() -> void;
auto pythia_deinit() -> void;

enum class Pythia_MPI_fn {
    Alltoall,
    Gather,
    Gatherv,
    Recv,
    Send,
};
auto constexpr pythia_fn_count = 2;
inline auto pythia_MPI_fn_name(Pythia_MPI_fn fn) {
    switch (fn) {
        case Pythia_MPI_fn::Send: return "MPI_Send";
        case Pythia_MPI_fn::Recv: return "MPI_Recv";
        case Pythia_MPI_fn::Alltoall: return "MPI_Alltoall";
        case Pythia_MPI_fn::Gather: return "MPI_Gather";
        case Pythia_MPI_fn::Gatherv: return "MPI_Gatherv";
        default: return "UNKNOWN";
    }
}

auto pythia_event(Pythia_MPI_fn fn, int arg1 = 0, int arg2 = 0) -> void;


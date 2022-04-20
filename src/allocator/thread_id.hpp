#pragma once

[[maybe_unused]] auto constexpr pythia_max_num_threads = 256;

auto pythia_new_thread_num() -> int;
auto pythia_set_thread_num(int) -> void;
auto pythia_get_thread_num() -> int;
auto pythia_get_num_threads() -> int;


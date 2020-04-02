#include <chrono>
#include <fstream>
#include <iostream>

#include "eta/core/ProgramOptions.hxx"
#include "eta/core/allocator.hpp"
#include "eta/core/rang.hpp"
#include "eta_trace/dot_export.hpp"
#include "eta_trace/factorization.hpp"

auto main(int argc, char ** argv) -> int {
    auto parser = po::parser {};

    auto & help_opt = parser["help"].abbreviation('h');
    help_opt.description("Print this help.");

    // auto & sep_opt = parser["separator"].abbreviation('s');
    // sep_opt.description("");

    // auto & bin_opt = parser["binary"].abbreviation('b');
    // bin_opt.description("");

    // auto & debug_opt = parser["debug"];
    // debug_opt.description("Enable debug output on stderr.");

    // auto & input_opt = parser[""].multi();
    // input_opt.description("");

    auto & file_opt = parser["file"].abbreviation('f').type(po::string);
    file_opt.description("Read in the file at the given path instead of readding on stdcin.");

    auto & dot_opt = parser["output-dot"].abbreviation('d').type(po::string);
    dot_opt.description("Write factorized trace in dot format in file at given path");

    auto & max_insertion_opt = parser["max-insertion-count"].type(po::u32).fallback(0u);
    max_insertion_opt.description("");

    if (!parser(argc, argv)) {
        std::cerr << "errors occurred; aborting\n";
        return -1;
    }

    if (help_opt.was_set()) {
        std::cout << argv[0]
                  << " read an input on the standard input and factorize it on the standard output"
                  << std::endl
                  << std::endl;
        std::cout << parser << std::endl;
        return 0;
    }

    auto const max_insertion = max_insertion_opt.get().u32;

    auto builder = TraceBuilder<StdAllocator> {};
    auto max_leaf_id = builder.newLeaf();

    auto tmp = static_cast<unsigned int>(0);
    auto i = 0u;
    auto duration = std::chrono::nanoseconds { 0u };

    std::cout << "index last_insert_duration full_factorization_duration node_count" << std::endl;

    auto parse = [&](auto & input) {
        while (input >> tmp && (max_insertion == 0u || i < max_insertion)) {
            while (tmp > max_leaf_id.value())
                max_leaf_id = builder.newLeaf();
            auto const t0 = std::chrono::high_resolution_clock::now();
            builder.insert(LeafId(tmp));
            auto const t1 = std::chrono::high_resolution_clock::now();
            auto const dt = t1 - t0;
            duration += dt;

            std::cout << i << ' ' << dt.count() << ' ' << duration.count() << ' '
                      << builder.trace().nodeCount() << std::endl;

            ++i;
        }
    };

    if (file_opt.was_set()) {
        auto input = std::ifstream {};
        input.open(file_opt.get().string);
        if (!input.is_open()) {
            std::cerr << "Failed to open input file \"" << file_opt.get().string << "\"." << std::endl;
            exit(1);
        }
        parse(input);

    } else {
        parse(std::cin);
    }

    if (dot_opt.was_set()) {
        auto output = std::ofstream {};
        output.open(dot_opt.get().string);
        if (!output.is_open()) {
            std::cerr << "Failed to open output file \"" << dot_opt.get().string << "\"." << std::endl;
            exit(1);
        }

        writeDotFile(
              output,
              builder.trace().root(),
              [](auto &&) { return ""; },
              [](auto && id) { return std::to_string(id.value()); });
    }
}

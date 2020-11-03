#include <chrono>
#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/factorization.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

#include "ProgramOptions.hxx"
#include "rang.hpp"

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

    std::string line;
    if (file_opt.was_set()) {
        getline(std::ifstream(file_opt.get().string), line);
    } else {
        getline(std::cin, line);
    }
    std::istringstream this_line(line);

    auto g = Grammar {};
    auto leafs = std::unordered_map<int, Terminal *> {};
    auto trace = std::vector<Terminal *> {};

    std::cout << "Loading" << std::endl;
    for (std::istream_iterator<int> begin(this_line), end; begin != end; ++begin) {
        auto it = leafs.find(*begin);
        if (it == leafs.end()) {
            trace.push_back(new_terminal(g, reinterpret_cast<void *>(*begin)));
            leafs.emplace_hint(it, std::pair { *begin, trace.back() });
        } else {
            trace.push_back(it->second);
        }
    }

    if (!trace.empty()) {
        std::cout << "Factorization" << std::endl;

        auto const t0 = std::chrono::high_resolution_clock::now();

        for (auto const n : trace)
            insertSymbol(g, n);

        auto const t1 = std::chrono::high_resolution_clock::now();
        auto const reduction_duration = std::chrono::duration<double>(t1 - t0);

        std::cout << "\nTrace of " << trace.size() << " events between " << leafs.size()
                  << " was reduced in " << reduction_duration.count() << "seconds." << std::endl;
    }
}

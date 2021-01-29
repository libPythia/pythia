#include <atomic>
#include <cassert>
#include <chrono>
#include <ctime>
#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/factorization.hpp>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#include "ProgramOptions.hxx"
#include "helpers.hpp"
#include "rang.hpp"

auto main(int argc, char ** argv) -> int {
    auto parser = po::parser {};

    // auto & duration_opt = parser["duration"].abbreviation('d').type(po::u32).fallback(5);
    // duration_opt.description("Maximum duration of the test in seconds. Default is 5 secondes");

    auto & max_error_count_opt = parser["max-errors"].abbreviation('e').type(po::u32).fallback(1);
    max_error_count_opt.description(
            "Print debug information instead of factorized trace. 0 for no "
            "limit. Default is 1.");

    // auto & thread_count_opt = parser["threads"].abbreviation('t').type(po::u32).fallback(1);
    // thread_count_opt.description("Number of threads to launch. Default is 1");

    auto & help_opt = parser["help"].abbreviation('h');
    help_opt.description("Print this help.");

    auto & print_valid_opt = parser["print-valid"].abbreviation('v');
    print_valid_opt.description("Print successfull test too.");

    auto & no_color_opt = parser["no-color"];
    no_color_opt.description("Don't use colors in output");

    auto & trace_size_opt = parser["trace-size"].abbreviation('s').type(po::u32).fallback(10);

    auto & alphabet_size_opt = parser["alphabet-size"].abbreviation('a').type(po::u32).fallback(4);

    auto & silent_opt = parser["silent"].abbreviation('S');

    if (!parser(argc, argv)) {
        std::cerr << "errors occurred; aborting\n";
        return -1;
    }

    if (help_opt.was_set()) {
        std::cout << parser << std::endl;
        return 0;
    }

    auto const max_error_count = max_error_count_opt.get().u32;
    // auto const max_duration = duration_opt.get().u32;
    // auto const thread_count = thread_count_opt.get().u32;
    auto const print_valid = print_valid_opt.was_set();
    auto const no_color = no_color_opt.was_set();
    auto const alphabet_size = alphabet_size_opt.get().u32;
    auto const trace_size = trace_size_opt.get().u32;
    auto const silent = silent_opt.was_set();

    // if (max_duration == 0u) {
    //     std::cerr << "Max duration cannot be null" << std::endl;
    //     return -1;
    // }

    // if (thread_count == 0u) {
    //     std::cerr << "Thread count cannot be null" << std::endl;
    //     return -1;
    // }

    auto error_count = std::atomic_uint { 0u };
    auto valid_count = std::atomic_uint { 0u };

    auto const start_time = std::chrono::system_clock::now();
    // auto const end_time = start_time + std::chrono::seconds(max_duration);
    auto last_time = start_time;

    // auto threads = std::vector<std::thread> {};
    // auto out_mutex = std::mutex {};

    auto reportError =
            [&](std::string const & input, std::string const & output, auto const & data) {
                ++error_count;
                if (silent)
                    return;

                // auto lock = std::unique_lock(out_mutex);
                if (!no_color)
                    std::cout << rang::fg::red << rang::style::bold;
                if (print_valid)
                    std::cout << '-';
                std::cout << input << ':';
                if (!no_color)
                    std::cout << rang::fg::reset << rang::style::reset;
                std::cout << output << " (" << data << ')' << std::endl;
            };

    auto reportSucess = [&](std::string const & input, std::string const & output) {
        // auto lock = std::unique_lock(out_mutex);
        if (!no_color)
            std::cout << rang::fg::green << rang::style::bold;
        std::cout << '+' << input << ':';
        if (!no_color)
            std::cout << rang::fg::reset << rang::style::reset;
        std::cout << output << std::endl;
    };

    auto const t0 = std::chrono::high_resolution_clock::now();
    auto trace = std::string(trace_size, 'a');
    while (true) {
        try {
            auto g = Grammar {};
            auto const root = build_grammar_from_string(g, trace);

            auto const output = get_string_from_grammar(root);

            if (!check_graph_integrity(g)) {
                reportError(trace, output, "Graph is corrupted");
            } else if (!check_grammar_constraints(g)) {
                reportError(trace, output, "Grammar constraints are violated.");
            } else if (output != trace) {
                reportError(trace, output, "Output and input differ.");
            } else {
                ++valid_count;
                auto const curr_time = std::chrono::system_clock::now();
                if (print_valid ||
                    std::chrono::duration<double>(curr_time - last_time).count() > 1.) {
                    reportSucess(trace, output);
                    last_time = curr_time;
                }
            }
        } catch (...) { reportError(trace, "", "{exception}"); }

        if (error_count == max_error_count)
            break;

        if ([&]() {
                auto i = 0u;
                while (i < trace_size) {
                    ++trace[i];
                    if (trace[i] == 'a' + std::min(alphabet_size, i + 1u)) {
                        trace[i] = 'a';
                        ++i;
                    } else
                        return false;
                }
                return true;
            }())
            break;
    }

    // for (auto & t : threads)
    //     t.join();

    auto const dt = std::chrono::high_resolution_clock::now() - t0;
    std::cerr << "Finished with " << error_count << " errors over " << (valid_count + error_count)
              << " tests in " << std::chrono::duration_cast<std::chrono::seconds>(dt).count()
              << "s." << std::endl;
}


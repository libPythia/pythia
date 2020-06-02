#include <atomic>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#ifdef assert
#    undef assert
#endif
#define assert(cond)   \
    id(!cond) throw 1; \
    else;

#include <eta/core/ProgramOptions.hxx>
#include <eta/core/allocator.hpp>
#include <eta/core/rang.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/factorization.hpp>

using namespace eta;
using namespace eta::factorization;

auto main(int argc, char ** argv) -> int {
    auto parser = po::parser {};

    auto & duration_opt = parser["duration"].abbreviation('d').type(po::u32).fallback(5);
    duration_opt.description("Maximum duration of the test in seconds. Default is 5 secondes");

    auto & max_error_count_opt = parser["max-errors"].abbreviation('e').type(po::u32).fallback(10);
    max_error_count_opt.description(
            "Print debug information instead of factorized trace. 0 for no "
            "limit. Default is 10.");

    auto & thread_count_opt = parser["threads"].abbreviation('t').type(po::u32).fallback(1);
    thread_count_opt.description("Number of threads to launch. Default is 1");

    auto & help_opt = parser["help"].abbreviation('h');
    help_opt.description("Print this help.");

    auto & print_valid_opt = parser["print-valid"].abbreviation('v');
    print_valid_opt.description("Print successfull test too.");

    auto & no_color_opt = parser["no-color"];
    no_color_opt.description("Don't use colors in output");

    auto & trace_size_opt = parser["trace-size"].abbreviation('s').type(po::u32).fallback(30);

    auto & alphabet_size_opt = parser["alphabet-size"].abbreviation('a').type(po::u32).fallback(10);

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
    auto const max_duration = duration_opt.get().u32;
    auto const thread_count = thread_count_opt.get().u32;
    auto const print_valid = print_valid_opt.was_set();
    auto const no_color = no_color_opt.was_set();
    auto const alphabet_size = alphabet_size_opt.get().u32;
    auto const trace_size = trace_size_opt.get().u32;
    auto const silent = silent_opt.was_set();

    if (max_duration == 0u) {
        std::cerr << "Max duration cannot be null" << std::endl;
        return -1;
    }

    if (thread_count == 0u) {
        std::cerr << "Thread count cannot be null" << std::endl;
        return -1;
    }

    auto error_count = std::atomic_uint { 0u };
    auto valid_count = std::atomic_uint { 0u };

    auto const start_time = std::chrono::system_clock::now();
    auto const end_time = start_time + std::chrono::seconds(max_duration);

    auto threads = std::vector<std::thread> {};
    auto out_mutex = std::mutex {};

    auto reportError = [&](auto const & trace, auto const & res) {
        ++error_count;
        if (silent)
            return;

        auto lock = std::unique_lock(out_mutex);
        if (!no_color)
            std::cout << rang::fg::red << rang::style::bold;
        if (print_valid)
            std::cout << '-';
        for (auto const c : trace)
            std::cout << (char)('a' + c);
        std::cout << ':';
        if (!no_color)
            std::cout << rang::fg::reset << rang::style::reset;
        if constexpr (std::is_same<std::decay_t<decltype(res)>, char const *>::value)
            std::cout << res;
        else if constexpr (std::is_same<decltype(res), std::string const &>::value)
            std::cout << res;
        else {
            for (auto const c : res)
                std::cout << (char)('a' + c.value());
        }
        std::cout << std::endl;
    };

    auto reportSucess = [&](auto const & trace, auto const & res) {
        auto lock = std::unique_lock(out_mutex);
        if (!no_color)
            std::cout << rang::fg::green << rang::style::bold;
        std::cout << '+';
        for (auto const c : trace)
            std::cout << (char)('a' + c);
        std::cout << ':';
        if (!no_color)
            std::cout << rang::fg::reset << rang::style::reset;
        std::cout << res << std::endl;
    };

    for (auto thread_id = 0u; thread_id < thread_count; ++thread_id) {
        threads.emplace_back([&]() {
            auto seed = std::chrono::system_clock::now().time_since_epoch().count() + thread_id;
            auto generator = std::default_random_engine(seed);
            while (std::chrono::system_clock::now() < end_time &&
                   (max_error_count == 0u || error_count < max_error_count)) {
                auto trace = std::vector<unsigned char>(trace_size);
                try {
                    auto distribution =
                            std::uniform_int_distribution<unsigned int> { 0, alphabet_size - 1u };

                    auto builder = TraceBuilder { std_allocator };
                    for (auto i = 0u; i < alphabet_size; ++i) {
                        [[maybe_unused]] auto const leafId = builder.newLeaf();
                        assert(leafId.value() == i);
                    }

                    for (auto & c : trace) {
                        c = distribution(generator);
                        builder.insert(LeafId(c));
                    }
                    auto const res = linearise(builder.trace().root());

                    auto isConform = [&](auto const & input, auto const & res) {
                        auto const size = res.size();
                        if (input.size() != size)
                            return false;
                        for (auto i = 0u; i < size; ++i)
                            if (res[i].value() != input[i])
                                return false;
                        return true;
                    };

                    auto isValid = [&](auto const & trace) {
                        auto size = trace.nodeCount();
                        for (auto i = 0u; i < size; ++i) {
                            auto const node = &trace[i];
                            if (node == trace.root()) {
                                if (node->previous != nullptr || node->isLeaf() ||
                                    node->son() == nullptr || node->parents.size() > 0u) {
                                    std::cerr << 1 << std::endl;
                                    return false;
                                }
                            } else if (node->isLeaf()) {
                                if (/*node->parents.size() == 0 ||*/ node->previous != nullptr ||
                                    node->next != nullptr || node->son() != nullptr) {
                                    std::cerr << 2 << ' ' << node->parents.size() << '[';
                                    for (auto && p : node->parents)
                                        std::cerr << p << ", ";
                                    std::cerr << "] " << node->previous << ' ' << node->next << ' '
                                              << node->son() << node->value().value() << std::endl;
                                    assert(false);
                                    return false;
                                }
                            } else if (node->son() == nullptr) {
                                // The node is invalid
                                if (node->parents.size() > 0 || node->previous != nullptr ||
                                    node->next != nullptr) {
                                    std::cerr << 3 << std::endl;
                                    return false;
                                }
                            } else if (node->previous == nullptr) {  // Node is pattern
                                auto occurences = 0u;
                                for (auto const parent : node->parents)
                                    occurences += parent->loop();
                                if (occurences < 2 || node->next == nullptr) {
                                    std::cerr << 4 << std::endl;
                                    return false;
                                }
                            } else if (node->parents.size() != 0) {
                                std::cerr << 5 << std::endl;
                                return false;
                            }
                        }
                        return true;
                    };

                    if (!isConform(trace, res)) {
                        reportError(trace, toStr(builder.trace().root(), true, [](auto id) {
                                        return std::string { (char)('a' + id.value()) };
                                    }));
                    } else if (!isValid(builder.trace())) {
                        reportError(trace, toStr(builder.trace().root(), true, [](auto id) {
                                        return std::string { (char)('a' + id.value()) };
                                    }));
                    } else {
                        ++valid_count;
                        if (print_valid) {
                            reportSucess(trace, toStr(builder.trace().root(), true, [](auto id) {
                                             return std::string { (char)('a' + id.value()) };
                                         }));
                        }
                    }
                } catch (...) { reportError(trace, "{exception}"); }
            }
        });
    }

    for (auto & t : threads)
        t.join();

    std::cerr << "Finished with " << error_count << " errors over " << (valid_count + error_count)
              << " tests." << std::endl;
}


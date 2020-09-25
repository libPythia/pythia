#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include <eta/factorization/allocator.hpp>
#include <eta/factorization/factorization.hpp>
#include "ProgramOptions.hxx"

using namespace eta;
using namespace eta::factorization;

struct params {
    long alphabet_size;
    long trace_size;
};

auto parseRange(std::string const & str) -> std::tuple<long, long, long> {
    auto number_count = 0u;
    std::string first, second, third;

    auto error = [&]() {
        std::cerr << "Invalid range \"" << str << "\"." << std::endl;
        exit(-1);
    };

    for (auto const c : str) {
        if (c == ':') {
            ++number_count;
        } else if (c >= '0' && c <= '9') {
            switch (number_count) {
                case 0: first += c; break;
                case 1: second += c; break;
                case 2: third += c; break;
                default: error();
            }
        } else
            error();
    }

    if (first.empty())
        error();

    if (number_count == 0) {
        auto const n = std::stol(first);
        return { n, 1l, n };
    } else if (number_count == 1) {
        if (second.empty())
            error();
        return { std::stol(first), 1l, std::stol(second) };
    } else if (number_count == 2) {
        if (second.empty() || third.empty())
            error();
        return { std::stol(first), std::stol(second), std::stol(third) };
    } else {
        error();
    }
    return { 0, 0, 0 };
}

auto main(int argc, char ** argv) -> int {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto parser = po::parser {};

    auto & help_opt = parser["help"].abbreviation('h');
    help_opt.description("Print this help.");

    auto & thread_count_opt = parser["threads"].abbreviation('t').type(po::u32).fallback(1);
    thread_count_opt.description("Number of threads to launch. Default is 1.");

    auto & trace_size_opt = parser["trace-size"].abbreviation('s').multi();
    trace_size_opt.type(po::string).fallback("1000:1000:100000");
    trace_size_opt.description("Range of trace size under format min:step:max.");

    auto & alphabet_size_opt = parser["alphabet-size"].abbreviation('a').multi();
    alphabet_size_opt.type(po::string).fallback("3:3:15");
    alphabet_size_opt.description("Range of alphabet size under format min:step:max.");

    auto & repeat_opt = parser["repeat"].abbreviation('r');
    repeat_opt.type(po::u32).fallback(1);
    repeat_opt.description("Number of times to repeat each mesure (default is 1).");

    if (!parser(argc, argv)) {
        std::cerr << "errors occurred; aborting\n";
        return -1;
    }

    if (help_opt.was_set()) {
        std::cout << parser << std::endl;
        return 0;
    }

    auto const thread_count = thread_count_opt.get().u32;

    // Prepare tests

    auto const configurations = [&]() {
        auto const repeat_count = repeat_opt.get().u32;

        auto res = std::vector<params> {};
        for (auto && trace_size : trace_size_opt)
            for (auto && alphabet_size : alphabet_size_opt) {
                auto const [min_size, step_size, max_size] = parseRange(trace_size.string);
                auto const [min_alpha, step_alpha, max_alpha] = parseRange(alphabet_size.string);
                for (auto trace_size = min_size; trace_size <= max_size; trace_size += step_size)
                    for (auto alphabet_size = min_alpha; alphabet_size <= max_alpha;
                         alphabet_size += step_alpha)
                        for (auto i = 0u; i < repeat_count; ++i)
                            res.push_back(params { alphabet_size, trace_size });
            }
        // std::reverse(res.begin(), res.end());  // shorter last for better workload repartition
        //                                        // on end of tests
        std::shuffle(res.begin(), res.end(), std::default_random_engine(seed));  // shuffle for
                                                                                 // better meaning
                                                                                 // of ponctual
                                                                                 // errors
        return res;
    }();

    // run tests

    auto out_mutex = std::mutex {};
    auto global_test_index = std::atomic_ulong { 0u };

    auto t_print = std::chrono::system_clock::now();

    auto threads = std::vector<std::thread> {};
    for (auto thread_id = 0u; thread_id < thread_count; ++thread_id) {
        threads.emplace_back([&] {
            auto generator = std::default_random_engine(seed + thread_id);

            while (true) {
                auto const test_index = global_test_index++;
                if (test_index >= configurations.size())
                    break;
                auto const config = configurations[test_index];

                auto builder = TraceBuilder { std_allocator };
                for (auto i = 0u; i < config.alphabet_size; ++i) {
                    [[maybe_unused]] auto const leafId = builder.newLeaf();
                    assert(leafId.value() == i);
                }

                auto distribution =
                        std::uniform_int_distribution<long> { 0, config.alphabet_size - 1u };

                auto duration = std::chrono::nanoseconds { 0u };
                for (auto i = 0u; i < config.trace_size; ++i) {
                    auto const t0 = std::chrono::high_resolution_clock::now();
                    builder.insert(LeafId(distribution(generator)));
                    auto const t1 = std::chrono::high_resolution_clock::now();
                    duration += t1 - t0;
                }

                auto lock = std::unique_lock(out_mutex);
                std::cout << config.alphabet_size << ' ' << config.trace_size << ' '
                          << duration.count() << ' ' << builder.trace().nodeCount() << std::endl;

                auto t = std::chrono::system_clock::now();
                if (t_print + std::chrono::seconds(5) < t) {
                    std::cerr << ((double)test_index / (double)configurations.size() * 100) << '%'
                              << std::endl;
                    t_print = t;
                }
            }
        });
    }

    for (auto & t : threads)
        t.join();
}


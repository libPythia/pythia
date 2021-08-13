
#include <atomic>
#include <cassert>
#include <chrono>
#include <ctime>
#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/reduction.hpp>
#include <iostream>
#include <mutex>
#include <optional>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

#include "ProgramOptions.hxx"
#include "helpers.hpp"
#include "rang.hpp"

// -----------------------------------------------------------------------

struct settings_t {
    unsigned int max_error_count;
    bool print_valid;
    bool no_color;
    char alphabet_size;
    unsigned int trace_size;
    bool silent;
    unsigned int thread_count;
};

// -----------------------------------------------------------------------

class Input final {
  public:
    Input() = default;
    auto operator=(Input const &) -> Input & = default;
    Input(char size, char alphabet_size) : _str(size, 'a'), _alphabet_size(alphabet_size) {}

    auto next() -> std::optional<std::string> {
        auto res = _str;
        for (auto & c : _str) {
            c += 1;
            if (c < 'a' + _alphabet_size)
                return res;
            c = 'a';
        }
        _str = "";
        return res;
    }

  private:
    std::string _str;
    char _alphabet_size;
};

// -----------------------------------------------------------------------

static auto error_count = std::atomic_uint { 0u };
static auto valid_count = std::atomic_uint { 0u };
static auto settings = settings_t {};
static auto common_input = Input {};
static auto input_mtx = std::mutex {};
static auto output_mtx = std::mutex {};

// -----------------------------------------------------------------------

auto parse_args(int argc, char ** argv) -> void {
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
        std::exit(-1);
    }

    if (help_opt.was_set()) {
        std::cout << parser << std::endl;
        std::exit(0);
    }

    settings.max_error_count = max_error_count_opt.get().u32;
    settings.print_valid = print_valid_opt.was_set();
    settings.no_color = no_color_opt.was_set();
    settings.alphabet_size = alphabet_size_opt.get().u32;
    settings.trace_size = trace_size_opt.get().u32;
    settings.silent = silent_opt.was_set();
    settings.thread_count = 8;
};

// -----------------------------------------------------------------------

auto report_success(std::string const & input) {
    ++valid_count;
    if (settings.silent)
        return;
    auto lock = std::unique_lock(output_mtx);
    if (!settings.no_color)
        std::cout << rang::fg::green << rang::style::bold;
    std::cout << '+' << input << ':';
    if (!settings.no_color)
        std::cout << rang::fg::reset << rang::style::reset;
    std::cout /* << output */ << std::endl;  // TODO
}

auto report_failure(std::string const & input) {
    ++error_count;
    if (settings.silent)
        return;

    auto lock = std::unique_lock(output_mtx);
    if (!settings.no_color)
        std::cout << rang::fg::red << rang::style::bold;
    if (settings.print_valid)
        std::cout << '-';
    std::cout << input << ':';
    if (!settings.no_color)
        std::cout << rang::fg::reset << rang::style::reset;
    std::cout /* << output << " (" << data << ')' */ << std::endl;  // TODO
}

auto test_input(std::string const & input) -> void {
    auto grammar = Grammar {};
    auto root = static_cast<NonTerminal *>(nullptr);

    for (auto i = 0; i < settings.alphabet_size; ++i)
        new_terminal(grammar, reinterpret_cast<void *>(static_cast<intptr_t>(i)));

    for (auto c : input)
        root = insertSymbol(grammar, root, grammar.terminals[c].get());

    report_success(input);  // TODO
}

auto worker() -> void {
    while (true) {
        auto input = std::optional<std::string> { std::nullopt };
        {
            auto const lck = std::unique_lock { input_mtx };
            input = common_input.next();
        }

        if (input.has_value() == false)
            return;

        test_input(input.value());
    }
}

auto main(int argc, char ** argv) -> int {
    parse_args(argc, argv);

    common_input = Input(settings.trace_size, settings.alphabet_size);

    auto const start_time = std::chrono::system_clock::now();

    auto threads = std::vector<std::thread> {};

    for (auto i = 0u; i < settings.thread_count; ++i)
        threads.emplace_back(worker);

    for (auto & t : threads)
        t.join();

    auto const duration = std::chrono::high_resolution_clock::now() - start_time;
    std::cerr << "Finished with " << error_count << " errors over " << (valid_count + error_count)
              << " tests in " << std::chrono::duration_cast<std::chrono::seconds>(duration).count()
              << "s over " << settings.thread_count << " threads." << std::endl;
}


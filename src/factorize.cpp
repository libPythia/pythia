#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define ETA_FACTO_PRINT(args)           \
    if (eta_factorization_print_debugs) \
        std::cerr << ((args));          \
    else {                              \
    }

bool eta_factorization_print_debugs = false;

#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/factorization.hpp>

#include "ProgramOptions.hxx"
#include "helpers.hpp"
#include "rang.hpp"

using namespace std::string_literals;

auto main(int argc, char ** argv) -> int {
    auto parser = po::parser {};

    // input settings

    // output settings

    auto & dump_dot_opt = parser["dot"].abbreviation('d');
    dump_dot_opt.description("Print result under dot format.");

    auto & display_grammar_opt = parser["grammar"].abbreviation('g');
    display_grammar_opt.description("Print result as a grammar.");

    auto & dump_debug_opt = parser["debug"];
    dump_debug_opt.description("Print debug information instead of factorized trace");

    auto & help_opt = parser["help"].abbreviation('h');
    help_opt.description("Print this help.");

    auto & check_opt = parser["check"].abbreviation('c');
    check_opt.description("Active checks on grammar produced");

    auto & no_color_opt = parser["no-color"];
    no_color_opt.description("Don't use color in ouput");

    // auto string_opt = parser["string"].abbreviation('s');
    // dump_debug_opt.description("Take each char of input is an event.");

    // auto separator_opt = parser["separator"].abbreviation('S');
    // separator_opt.description("Set the separator to use when parsing input (default is space)");

    auto & print_input_opt = parser["print_input"].abbreviation('i');
    print_input_opt.description("For each output, print the input at the begining of the line");

    auto & input_opt = parser[""].multi();
    input_opt.description("The input as a string");

    if (!parser(argc, argv)) {
        std::cerr << "errors occurred; aborting\n";
        return -1;
    }

    if (help_opt.was_set()) {
        std::cout << parser << std::endl;
        return 0;
    }

    auto const input_from_stdcin = !input_opt.was_set();
    auto const dump_dot_format = dump_dot_opt.was_set();
    auto const display_grammar = display_grammar_opt.was_set();
    eta_factorization_print_debugs = dump_debug_opt.was_set();
    auto const print_input = print_input_opt.was_set();
    auto const no_color = no_color_opt.was_set();
    auto const check = check_opt.was_set();

    // TODO incompatibilités

    auto for_each_input = [&](auto && f) {
        auto b = false;
        if (input_from_stdcin) {
            for (std::string input; std::getline(std::cin, input);) {
                if (input != "") {
                    b = true;
                    f(input);
                }
            }
        } else {
            for (auto && input : input_opt) {
                if (input.string != "") {
                    b = true;
                    f(input.string);
                }
            }
        }
        if (!b)
            std::cerr << "\nNo input" << std::endl;
        ;
    };

    for_each_input([&](std::string const & input) {
        auto g = Grammar {};

        build_grammar_from_string(g, input);

        if (print_input) {
            if (!no_color)
                std::cout << rang::fg::blue << rang::style::bold;
            std::cout << input << ':';
            if (!no_color)
                std::cout << rang::style::reset << rang::fg::reset;
        }

        if (display_grammar) {
            print_grammar(g, std::cout, [](Terminal const * t, std::ostream & os) {
                os << static_cast<char>(reinterpret_cast<size_t>(t->payload));
            });
        } else if (dump_dot_format) {
            print_dot_file(
                    g,
                    std::cout,
                    [](Terminal const * t, std::ostream & os) {
                        os << static_cast<char>(reinterpret_cast<size_t>(t->payload));
                    },
                    print_input);
        } else {
            print_reduced_trace(g, std::cout, [](Terminal const * t, std::ostream & os) {
                os << static_cast<char>(reinterpret_cast<size_t>(t->payload));
            });
        }

        if (check) {
            if (!check_grammar_constraints(g)) {
                if (!no_color)
                    std::cout << rang::fg::red << rang::style::bold;
                std::cout << "\nGrammar constraints not respected.";
                if (!no_color)
                    std::cout << rang::style::reset << rang::fg::reset;
            }
            if (!check_graph_integrity(g)) {
                if (!no_color)
                    std::cout << rang::fg::red << rang::style::bold;
                std::cout << "\nGraph integrity is compromised.";
                if (!no_color)
                    std::cout << rang::style::reset << rang::fg::reset;
            }
            if (get_string_from_grammar(g) != input) {
                if (!no_color)
                    std::cout << rang::fg::red << rang::style::bold;
                std::cout << "\nInput and linearised trace from grammar are different.";
                if (!no_color)
                    std::cout << rang::style::reset << rang::fg::reset;
            }
        }

        std::cout << std::endl;
    });
}


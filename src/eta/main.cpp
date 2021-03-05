#include <eta/core/colors.hpp>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/export.hpp>
#include <iostream>

#include "errors.hpp"
#include "input.hpp"
#include "settings.hpp"

auto compare(Grammar const & g, std::string const & str, terminal_printer const & printer) -> void;

auto main(int argc, char ** argv) -> int {
    auto const settings = parse_settings(argc, argv);

    auto input = read_input(settings);

    auto print_terminal = [](Terminal const * t, std::ostream & os) {
        os << Data::get_printable_data(t);
    };

    auto print_input = [&](NonTerminal const * nt) {
        if (settings.print_input) {
            eta::set_color(std::cout, eta::color_t::green);
            print_trace(nt, std::cout, print_terminal);
            eta::set_color(std::cout, eta::color_t::standard);
            std::cout << ": ";
        }
    };

    switch (settings.output_mode) {
        case output_t::binary: break;

        case output_t::dot: {
            print_dot_file(input.grammar, std::cout, print_terminal, settings.print_input);
        } break;
        case output_t::expend: {
            for (auto const thread : input.threads) {
                print_input(thread);
                print_trace(thread, std::cout, print_terminal);
            }
        } break;
        case output_t::grammar:
            for (auto const thread : input.threads)
                print_input(thread);
            print_grammar(input.grammar, std::cout, print_terminal);
            break;
        case output_t::reduced:
            for (auto const thread : input.threads) {
                print_input(thread);
                print_reduced_trace(thread, std::cout, print_terminal);
                std::cout << std::endl;
            }
            break;
        case output_t::terminals:
            for (auto const & terminal : input.grammar.terminals) {
                auto const occurrences_count = terminal->occurrences_without_successor.size() +
                                               terminal->occurrences_with_successor.size();
                if (occurrences_count > 0) {
                    print_terminal(terminal.get(), std::cout);
                    std::cout << std::endl;
                }
            }
            break;
    }

    if (settings.compare != "") {
        compare(input.grammar, settings.compare, print_terminal);
        std::cout << std::endl;
    }
}


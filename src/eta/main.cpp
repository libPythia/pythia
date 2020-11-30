#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/factorization.hpp>
#include <iostream>

#include "colors.hpp"
#include "errors.hpp"
#include "input.hpp"
#include "settings.hpp"

auto main(int argc, char ** argv) -> int {
    auto const settings = parse_settings(argc, argv);

    if (settings.output_mode == output_t::dot)
        print_dot_file_begin(std::cout);

    auto input_count = 0u;
    while (true) {
        auto grammar = Grammar {};
        auto state = get_input(grammar, settings);

        if (state == input_state::none)
            break;
        ++input_count;
        if (input_count > 1 && settings.output_mode == output_t::binary) {
            print_error("Two many inputs for output mode");
            exit(errors_t::INPUT_OUTPUT_MISMATCH);
        }

        auto print_terminal = [](Terminal const * t, std::ostream & os) {
            os << static_cast<char>(reinterpret_cast<size_t>(t->payload));
        };

        auto print_input = [&]() {
            if (settings.print_input) {
                set_color(std::cout, color_t::green);
                print_trace(grammar, std::cout, print_terminal);
                set_color(std::cout, color_t::standard);
                std::cout << ": ";
            }
        };

        switch (settings.output_mode) {
            case output_t::reduced: {
                print_input();
                print_reduced_trace(grammar, std::cout, print_terminal);
            } break;
            case output_t::dot: {
                print_dot_file_grammar(grammar, std::cout, print_terminal, settings.print_input);
            } break;
            case output_t::grammar: {
                print_input();
                print_grammar(grammar, std::cout, print_terminal);
            } break;
            case output_t::binary: {
                print_bin_file(grammar, std::cout, print_terminal);
            } break;
        }

        std::cout << std::endl;
        if (state == input_state::last)
            break;
    }

    if (settings.output_mode == output_t::dot)
        print_dot_file_end(std::cout);
}


#include "input.hpp"

#include <unistd.h>

#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/check.hpp>

#include "errors.hpp"

static auto is_printable(char c) -> bool { return (c >= 32 && c <= 126); }

static auto integrity_checks(Grammar const & grammar, settings_t const & settings) -> void {
    print_debug(settings, "Performs integrity checks in grammar representation.");
    if (!check_graph_integrity(grammar)) {
        print_error("Graph integrity is compromised.");
        exit(errors_t::CHECK_FAILED);
    }
    print_debug(settings, "Performs checks of contrains over the grammar.");
    if (!check_grammar_constraints(grammar)) {
        print_error("Grammar constraints not respected.");
        exit(errors_t::CHECK_FAILED);
    }
}

auto get_input(Grammar & g, settings_t const & settings) -> input_state {
    print_debug(settings, "Input was reduced");

    switch (settings.input_mode) {
        case input_t::lines: {
            print_debug(settings, "Use line input.");
            auto terminals = std::vector<Terminal *>(256, nullptr);
            auto input = std::vector<Terminal *> {};
            char c;
            auto input_len = 0;
            while (read(0, &c, sizeof(c)) == sizeof(c)) {
                if (c == '\n')
                    break;
                if (!is_printable(c))
                    continue;
                ++input_len;
                auto const terminal = [&]() {
                    auto & res = terminals[c];
                    if (res == nullptr)
                        res = new_terminal(g, Data::make_terminal_data(c));
                    return res;
                }();
                if (settings.check)
                    input.push_back(terminal);
                insertSymbol(g, terminal);
            }
            if (input_len == 0)
                return input_state::none;

            print_debug(settings, "Input length = ", input_len, '.');
            if (settings.check) {
                print_debug(settings,
                            "Perform conformity check between input and reduced grammar.");
                auto const output = linearise_grammar(g);
                if (![&]() {
                        auto it1 = input.begin();
                        auto it2 = output.begin();
                        auto const end1 = input.end();
                        auto const end2 = output.end();
                        for (; it1 != end1 && it2 != end2; ++it1, ++it2)
                            if (*it1 != *it2)
                                return false;
                        return it1 == end1 || it2 == end2;
                    }()) {
                    print_error("Input and output differs.");
                    exit(errors_t::CHECK_FAILED);
                }

                integrity_checks(g, settings);
            }

            return input_state::yes;
        } break;
        case input_t::text: [[fallthrough]];
        case input_t::non_printable: {
            if (settings.input_mode == input_t::text)
                print_debug(settings, "Use text input without non printable characters.");
            else
                print_debug(settings, "Use text input with non printable characters.");
            auto terminals = std::vector<Terminal *>(256, nullptr);
            auto input = std::vector<Terminal *> {};
            char c;
            auto input_len = 0;
            while (read(0, &c, sizeof(c)) == sizeof(c)) {
                if (settings.input_mode == input_t::text && !is_printable(c))
                    continue;
                ++input_len;
                auto const terminal = [&]() {
                    auto & res = terminals[c];
                    if (res == nullptr)
                        res = new_terminal(g, Data::make_terminal_data(c));
                    return res;
                }();
                if (settings.check)
                    input.push_back(terminal);
                insertSymbol(g, terminal);
            }

            print_debug(settings, "Input length = ", input_len, '.');
            if (settings.check) {
                print_debug(settings,
                            "Perform conformity check between input and reduced grammar.");
                auto const output = linearise_grammar(g);
                if (![&]() {
                        auto it1 = input.begin();
                        auto it2 = output.begin();
                        auto const end1 = input.end();
                        auto const end2 = output.end();
                        for (; it1 != end1 && it2 != end2; ++it1, ++it2)
                            if (*it1 != *it2)
                                return false;
                        return it1 == end1 || it2 == end2;
                    }()) {
                    print_error("Input and output differs.");
                    exit(errors_t::CHECK_FAILED);
                }

                integrity_checks(g, settings);
            }
            return input_state::last;
        } break;
        case input_t::binary: {
            load_bin_file(g, std::cin);

            if (settings.check)
                integrity_checks(g, settings);

            return input_state::last;
        } break;
    }
    return input_state::none;
}

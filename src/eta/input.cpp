#include "input.hpp"

#include <unistd.h>

#include <cassert>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <fstream>
#include <sstream>

#include "errors.hpp"

static auto is_printable(char c) -> bool { return (c >= 32 && c <= 126); }

template <typename F> static auto for_each_byte(std::istream & is, F && f) -> void {
    while (true) {
        auto c = char {};
        is.read(&c, sizeof(c));
        if (!is)
            break;
        f(c);
    }
}

static auto read_input(settings_t const & settings,
                       Input & res,
                       std::vector<Terminal *> & terminals,
                       std::istream & is) -> void {
    auto new_thread = true;

    auto for_each_byte = [](auto & is, auto && f) -> void {
        while (true) {
            auto c = char {};
            is.read(&c, sizeof(c));
            if (!is)
                break;
            f(c);
        }
    };

    auto insert_symbol = [&res, &terminals, &new_thread](auto c) -> void {
        if (new_thread) {
            res.threads.emplace_back(nullptr);
            new_thread = false;
        }
        auto & nt = res.threads.back();
        auto & t = terminals[c];
        if (t == nullptr)
            t = new_terminal(res.grammar, Data::make_terminal_data(c));
        nt = insertSymbol(res.grammar, nt, t);
    };

    switch (settings.input_mode) {
        case input_t::binary: {
            load_bin_file(res.grammar, is);
            for (auto const & nonterminal : res.grammar.nonterminals.in_use_nonterminals()) {
                if (occurrences_count(nonterminal) == 0)
                    res.threads.emplace_back(const_cast<NonTerminal *>(nonterminal));
            }
            break;
        }
        case input_t::lines:
            for_each_byte(is, [&](auto c) {
                if (c == '\n')
                    new_thread = true;
                else if (is_printable(c))
                    insert_symbol(c);
            });
            break;
        case input_t::non_printable: for_each_byte(is, [&](auto c) { insert_symbol(c); }); break;
        case input_t::text:
            for_each_byte(is, [&](auto c) {
                if (is_printable(c))
                    insert_symbol(c);
            });
            break;
    }
}

auto read_input(settings_t const & settings) -> Input {
    auto terminals = std::vector<Terminal *>(256, nullptr);
    auto res = Input {};

    switch (settings.input_src) {
        case input_src_t::argument: {
            for (auto const & input : settings.input_data) {
                auto ss = std::stringstream {};
                ss << input;
                read_input(settings, res, terminals, ss);
            }
        } break;
        case input_src_t::file: {
            for (auto const & input_file : settings.input_data) {
                auto file = std::ifstream {};
                file.open(input_file, std::fstream::binary);
                if (!file.is_open()) {
                    file.open(input_file);
                    if (!file.is_open()) {
                        print_error("Failed to open file.");
                        exit(errors_t::FAILED_TO_OPEN_INPUT_FILE);
                    }
                }
                read_input(settings, res, terminals, file);
            }
        } break;
        case input_src_t::std_in: {
            read_input(settings, res, terminals, std::cin);
        } break;
        default: assert(false);
    }

    return res;
}


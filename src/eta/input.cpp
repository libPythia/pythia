#include "input.hpp"

#include <unistd.h>

#include <cassert>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <fstream>

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

static auto read_input(settings_t const & settings, std::istream & is) -> Input {
    auto res = Input {};
    res.threads.emplace_back(nullptr);

    auto for_each_byte = [&is](auto && f) -> void {
        while (true) {
            auto c = char {};
            is.read(&c, sizeof(c));
            if (!is)
                break;
            f(c);
        }
    };

    auto terminals = std::vector<Terminal *>(256, nullptr);
    auto insert_symbol = [&res, &terminals](auto c, auto & nt) -> void {
        auto & t = terminals[c];
        if (t == nullptr)
            t = new_terminal(res.grammar, Data::make_terminal_data(c));
        nt = insertSymbol(res.grammar, nt, t);
    };

    switch (settings.input_mode) {
        case input_t::binary: {
            load_bin_file(res.grammar, is);
            for (auto const & nonterminal : res.grammar.nonterminals.in_use_nonterminals()) {
                if (occurences_count(nonterminal) == 0)
                    res.threads.emplace_back(const_cast<NonTerminal *>(nonterminal));
            }
            break;
        }
        case input_t::lines:
            for_each_byte([&](auto c) {
                if (c == '\n')
                    res.threads.emplace_back(nullptr);
                else if (is_printable(c))
                    insert_symbol(c, res.threads.back());
            });
            break;
        case input_t::non_printable:
            for_each_byte([&](auto c) { insert_symbol(c, res.threads.back()); });
            break;
        case input_t::text:
            for_each_byte([&](auto c) {
                if (is_printable(c))
                    insert_symbol(c, res.threads.back());
            });
            break;
    }

    return res;
}

auto read_input(settings_t const & settings) -> Input {
    if (settings.input_file != "") {
        auto input_file = std::fstream {};
        input_file.open(settings.input_file, std::fstream::binary);
        if (!input_file.is_open()) {
            input_file.open(settings.input_file);
            if (!input_file.is_open()) {
                print_error("Failed to open file.");
                exit(errors_t::FAILED_TO_OPEN_INPUT_FILE);
            }
        }
        return read_input(settings, input_file);
    } else {
        return read_input(settings, std::cin);
    }
}


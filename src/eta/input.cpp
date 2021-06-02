#include "input.hpp"

#include <unistd.h>

#include <cassert>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/check.hpp>
#include <eta/factorization/export.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

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

template <typename F> static auto for_each_line(std::istream & is, F && f) -> void {
    auto line = std::string {};
    while (std::getline(is, line)) {
        f(std::move(line));
    }
}

template <typename F>
static auto insert_symbol(Input & res, std::string str, F && get_terminal) -> void {
    auto & nt = res.threads.back();
    nt = insertSymbol(res.grammar, nt, get_terminal(std::move(str)));
}

template <typename F> static auto insert_symbol(Input & res, char c, F && get_terminal) -> void {
    char buf[2] = { c, 0 };
    insert_symbol(res, buf, get_terminal);
}

static auto new_thread(Input & res) -> void { res.threads.emplace_back(nullptr); }

template <typename F>
static auto read_input(settings_t const & settings,
                       Input & res,
                       F && get_terminal,
                       std::istream & is) -> void {
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
            for_each_line(is, [&](auto line) {
                new_thread(res);
                for (auto const c : line)
                    if (is_printable(c))
                        insert_symbol(res, c, get_terminal);
            });
            break;
        case input_t::non_printable:
            new_thread(res);
            for_each_byte(is, [&](auto c) { insert_symbol(res, c, get_terminal); });
            break;
        case input_t::text:
            new_thread(res);
            for_each_byte(is, [&](auto c) {
                if (is_printable(c))
                    insert_symbol(res, c, get_terminal);
            });
            break;
        case input_t::logs: {
            new_thread(res);
            for_each_line(is, [&](auto line) { insert_symbol(res, line, get_terminal); });
        } break;
    }
}

auto read_input(settings_t const & settings) -> Input {
    auto terminals = std::unordered_map<std::string, Terminal *> {};
    auto res = Input {};

    auto const get_terminal = [&terminals, &res](std::string str) -> Terminal * {
        auto const it = terminals.find(str);
        if (it == terminals.end()) {
            auto const nt = new_terminal(res.grammar, strdup(str.c_str()));
            terminals.try_emplace(it, str, nt);
            return nt;
        }
        return it->second;
    };

    switch (settings.input_src) {
        case input_src_t::argument: {
            for (auto const & input : settings.input_data) {
                auto ss = std::stringstream {};
                ss << input;
                read_input(settings, res, get_terminal, ss);
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
                read_input(settings, res, get_terminal, file);
            }
        } break;
        case input_src_t::std_in: {
            read_input(settings, res, get_terminal, std::cin);
        } break;
        default: assert(false);
    }

    return res;
}


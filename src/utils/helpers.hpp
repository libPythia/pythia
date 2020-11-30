#pragma once

#include <eta/factorization/factorization.hpp>
#include <string>
#include <vector>

static auto build_grammar_from_string(Grammar & g, std::string const & input) {
    auto terminals = std::vector<Terminal *>(256, nullptr);
    for (auto const c : input) {
        auto terminal = [&]() -> Terminal * {
            if (terminals[c] == nullptr)
                terminals[c] = new_terminal(g, reinterpret_cast<void *>(c));
            return terminals[c];
        }();

        insertSymbol(g, terminal);
    }
}

static auto get_string_from_grammar(Grammar const & g) -> std::string {
    auto const trace = linearise_grammar(g);
    auto res = std::string {};

    for (auto const n : trace) {
        res.push_back(static_cast<char>(reinterpret_cast<size_t>(n->payload)));
    }

    return res;
}


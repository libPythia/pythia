#pragma once

#include <eta/factorization/factorization.hpp>
#include <string>
#include <vector>

static auto build_grammar_from_string(Grammar & g, std::string const & input)
        -> NonTerminal const * {
    auto terminals = std::vector<Terminal *>(256, nullptr);
    auto root = static_cast<NonTerminal *>(nullptr);

    for (auto const c : input) {
        auto terminal = [&]() -> Terminal * {
            if (terminals[c] == nullptr)
                terminals[c] = new_terminal(g, reinterpret_cast<void *>(c));
            return terminals[c];
        }();

        root = insertSymbol(g, root, terminal);
    }

    return root;
}

static auto get_string_from_grammar(NonTerminal const * root) -> std::string {
    auto const trace = linearise_grammar(root);
    auto res = std::string {};

    for (auto const n : trace) {
        res.push_back(static_cast<char>(reinterpret_cast<size_t>(n->payload)));
    }

    return res;
}


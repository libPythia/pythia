#pragma once

#include <cstring>
#include <eta/factorization/factorization.hpp>

struct Data {
    Grammar grammar;
    NonTerminal * root = nullptr;

    static auto make_terminal_data(char c) -> void * {
        auto res = static_cast<char *>(std::malloc(2));
        res[0] = c;
        res[1] = '\0';
        return static_cast<void *>(res);
    }

    static auto make_terminal_data(char const * str, int size = -1) -> void * {
        if (size < 0)
            size = strlen(str);
        auto res = static_cast<char *>(std::malloc(size + 1));
        memcpy(res, str, size);
        res[size] = '\0';
        return static_cast<void *>(res);
    }

    static auto get_printable_data(Terminal const * terminal) -> char const * {
        return static_cast<char const *>(terminal->payload);
    }

    ~Data() {
        for (auto const & terminal : grammar.terminals) {
            std::free(terminal->payload);
        }
    }
};


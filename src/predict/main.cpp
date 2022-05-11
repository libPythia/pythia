#include <array>
#include <eta/factorization/reduction.hpp>
#include <iostream>

auto main(int argc, char ** argv) -> int {
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage : " << argv[0] << " trace input [count]\n";
        return 1;
    }

    auto const trace = std::string(argv[1]);
    auto const input = std::string(argv[2]);
    auto const count = [&]() {
        auto c = 1u;
        if (argc > 3)
            sscanf(argv[3], "%u", &c);
        return c;
    }();

    auto grammar = Grammar {};
    auto terminals = std::array<Terminal const *, 128> {};
    auto root = static_cast<NonTerminal *>(nullptr);
    for (auto i = 0; i < 128; ++i)
        terminals[i] = new_terminal(grammar, reinterpret_cast<void *>(i));

    for (auto const i : trace)
        root =

                std::cout << "Trace : '" << trace << "'\n";
    std::cout << "Input : '" << input << "'\n";
    std::cout << "Count : " << count << std::endl;
}


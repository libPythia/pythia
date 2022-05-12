#include <array>
#include <eta/factorization/export.hpp>
#include <eta/factorization/prediction.hpp>
#include <eta/factorization/reduction.hpp>
#include <iostream>

static auto print_terminal(Terminal const * t, std::ostream & os) { os << *(char *)&t->payload; }

auto main(int argc, char ** argv) -> int {
    // Get parameters
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

    // Reduce trace
    auto grammar = Grammar {};
    auto terminals = std::array<Terminal *, 128> {};
    auto root = static_cast<NonTerminal *>(nullptr);
    for (auto i = 0; i < 128; ++i)
        terminals[i] = new_terminal(grammar, reinterpret_cast<void *>(i));

    for (auto const c : trace)
        root = append_terminal(grammar, root, terminals[c]);
    std::cout << "Trace : '";
    print_reduced_trace(root, std::cout, print_terminal);
    std::cout << std::endl;

    // compare the input to the trace

    auto estimation = init_estimation_from_start(grammar);
    for (auto const c : input) {
        estimation = next_estimation(std::move(estimation), terminals[c]);
        std::cout << "New input '" << c << "' leaves " << estimation.size() << " possibilities.\n";
    }

    // Predict next events
    auto dig_in_futur = [count](Prediction p, auto depth, auto rec) -> void {
        if (depth == count)
            return;
        do {
            for (auto i = 0u; i < depth; ++i)
                std::cout << "    ";
            std::cout << " - ";
            print_terminal(get_terminal(p), std::cout);
            std::cout << std::endl;
            auto np = p;
            if (get_first_next(&np))
                rec(std::move(np), depth + 1, rec);
        } while (get_alternative(&p));
    };
    auto prediction = get_prediction_from_estimation(estimation);
    if (prediction.index < prediction.estimation.size())
        dig_in_futur(std::move(prediction), 0u, dig_in_futur);
    else
        std::cout << "No prediction is available" << std::endl;

    // Print result

    // std::cout << "Input : '" << input << "'\n";
    // std::cout << "Count : " << count << std::endl;
}


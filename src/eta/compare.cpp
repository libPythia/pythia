#include <cassert>
#include <eta/core/colors.hpp>
#include <eta/factorization/export.hpp>
#include <eta/factorization/prediction.hpp>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------

static auto print_symbol(Symbol const * s, terminal_printer const & printer) -> void {
    if (is_terminal(s)) {
        printer(as_terminal(s), std::cerr);
    } else {
        auto const nonterminal = as_nonterminal(s);
        auto n = nonterminal->first;
        while (true) {
            print_symbol(n->maps_to, printer);
            if (!is_node(n->next))
                break;
            n = as_node(n->next);
        }
    }
}

// -----------------------------------------------------------

auto compare(Grammar & g, std::string const & str, terminal_printer const & printer) -> void {
    std::cout << std::endl;

    auto const trace = [&]() {
        auto index = std::unordered_map<char, Terminal const *> {};
        for (auto const & terminal : g.terminals) {
            auto ss = std::stringstream {};
            printer(terminal.get(), ss);
            assert(ss.str().size() == 1);
            index.emplace(ss.str()[0], terminal.get());
        }

        auto tmp = std::vector<std::pair<Terminal const *, char>> {};
        for (auto const c : str) {
            auto const it = index.find(c);
            if (it != index.end())
                tmp.emplace_back(it->second, c);
            else
                tmp.emplace_back(nullptr, c);
        }
        return tmp;
    }();

    auto e = Estimation {};
    auto fg = buildFlowGraph(g);

    init_estimation(&e, &fg);

    auto do_prediction = [&printer](auto rec, auto prediction, auto depth) -> void {
        auto first = true;
        do {
            if (first) {
                first = false;
            } else
                std::cout << ", ";

            print_symbol(get_terminal(prediction), printer);
            std::cout << ' ' << static_cast<int>(get_probability(prediction) * 100.) << '%';

            if (depth > 0) {
                auto child_prediction = Prediction {};
                copy_prediction(&child_prediction, prediction);
                if (get_prediction_tree_child(&child_prediction)) {
                    std::cout << " (";
                    rec(rec, &child_prediction, depth - 1);
                    std::cout << ')';
                }
            }

        } while (get_prediction_tree_sibling(prediction));
    };

    for (auto const & it : trace) {
        update_estimation(&e, it.first);
        std::cout << "# Update " << it.second << std::endl;
        auto p = Prediction {};
        if (reset_prediction(&p, &e)) {
            std::cout << "-> Predict ";
            do_prediction(do_prediction, &p, 3);
        }

        std::cout << std::endl;
    }

    eta::set_color(std::cout, eta::color_t::standard);
    eta::set_style(std::cout, eta::style_t::standard);
}

#include <eta/core/colors.hpp>
#include <eta/factorization/bin_file.hpp>
#include <eta/factorization/export.hpp>
#include <iostream>
#include <sstream>
#include <string>

#include "errors.hpp"
#include "input.hpp"
#include "settings.hpp"

#ifdef ETA_GUI_ENABLED
#    include "eta_gui.h"
#    include "imgui.h"
#endif

static auto print_tree(
        std::ostream & os,
        Symbol const * symbol,
        terminal_printer const & printer,
        size_t indent,
        std::unordered_map<NonTerminal const *, std::string> const & non_terminal_names) -> void {
    auto const temporal_checkpoint = indent <= 1 && occurrences_count(symbol) == 1;
    if (is_terminal(symbol)) {
        os << '"';
        if (temporal_checkpoint)
            set_color(os, eta::color_t::red);
        else
            set_color(os, eta::color_t::blue);
        printer(as_terminal(symbol), os);
        set_color(os, eta::color_t::standard);
        os << '"';
    } else {
        if (temporal_checkpoint) {
            set_color(os, eta::color_t::red);
            set_style(os, eta::style_t::bold);
            os << "* ";
            set_style(os, eta::style_t::standard);
            set_color(os, eta::color_t::standard);
        }
        auto const nonterminal = as_nonterminal(symbol);
        set_color(os, eta::color_t::green);
        os << non_terminal_names.at(nonterminal);
        set_color(os, eta::color_t::standard);
        auto node = nonterminal->first;
        while (true) {
            os << std::endl;
            for (auto i = 0u; i < indent; ++i)
                os << "    ";
            set_color(os, eta::color_t::cyan);

            os << "- ";
            if (node->repeats > 1)
                os << node->repeats << "x ";
            set_color(os, eta::color_t::standard);
            print_tree(os, node->maps_to, printer, indent + 1, non_terminal_names);

            if (!is_node(node->next))
                break;
            node = as_node(node->next);
        }
    }
}

static auto build_non_terminal_names(Grammar const & g)
        -> std::unordered_map<NonTerminal const *, std::string> {
    auto res = std::unordered_map<NonTerminal const *, std::string> {};
    auto tmp = std::string {};
    auto incr = [&tmp](auto rec, int index) -> void {
        if (index == -1) {
            tmp = "A" + tmp;
        } else if (tmp[index] == 'Z') {
            tmp[index] = 'A';
            rec(rec, index - 1);
        } else {
            ++tmp[index];
        }
    };
    for (auto const & nonterminal : g.nonterminals.in_use_nonterminals()) {
        incr(incr, tmp.size() - 1);
        res.emplace(nonterminal, tmp);
    }
    return res;
}

auto main(int argc, char ** argv) -> int {
    auto const settings = parse_settings(argc, argv);

    auto input = read_input(settings);

    auto print_terminal = [](Terminal const * t, std::ostream & os) {
        os << Data::get_printable_data(t);
    };

    auto print_input = [&](NonTerminal const * nt) {
        if (settings.print_input) {
            eta::set_color(std::cout, eta::color_t::green);
            print_trace(nt, std::cout, print_terminal);
            eta::set_color(std::cout, eta::color_t::standard);
            std::cout << ": ";
        }
    };

    switch (settings.output_mode) {
        case output_t::binary: print_bin_file(input.grammar, std::cout, print_terminal); break;
        case output_t::dot: {
            print_dot_file(input.grammar, std::cout, print_terminal, settings.print_input);
        } break;
        case output_t::expend: {
            for (auto const thread : input.threads) {
                print_input(thread);
                print_trace(thread, std::cout, print_terminal);
            }
        } break;
        case output_t::grammar:
            for (auto const thread : input.threads)
                print_input(thread);
            print_grammar(input.grammar, std::cout, print_terminal);
            break;
        case output_t::reduced:
            for (auto const thread : input.threads) {
                print_input(thread);
                print_reduced_trace(thread, std::cout, print_terminal);
                std::cout << std::endl;
            }
            break;
        case output_t::tree: {
            auto const non_terminal_names = build_non_terminal_names(input.grammar);
            for (auto const & root : input.threads)
                print_tree(std::cout, as_symbol(root), print_terminal, 0, non_terminal_names);
            std::cout << std::endl;
        } break;
        case output_t::terminals:
            for (auto const & terminal : input.grammar.terminals) {
                auto const occurrences_count = terminal->occurrences_without_successor.size() +
                                               terminal->occurrences_with_successor.size();
                if (occurrences_count > 0) {
                    print_terminal(terminal.get(), std::cout);
                    std::cout << std::endl;
                }
            }
            break;
#ifdef ETA_GUI_ENABLED
        case output_t::gui: {
            auto const non_terminal_name = build_non_terminal_names(input.grammar);
            eta_gui([&]() {
                auto const print_symbol = [&](auto rec, NonTerminal const * nonterminal) -> void {
                    auto node = nonterminal->first;
                    while (true) {
                        auto ss = std::stringstream {};
                        if (is_terminal(node->maps_to)) {
                            ss << node->repeats << "x \""
                               << static_cast<char const *>(as_terminal(node->maps_to)->payload)
                               << '"';
                            ImGui::Text(ss.str().c_str());
                        } else {
                            ImGui::Indent();
                            ss << node->repeats << "x "
                               << non_terminal_name.at(as_nonterminal(node->maps_to)) << "##"
                               << node;
                            auto const open = ImGui::TreeNode(ss.str().c_str());
                            ImGui::SameLine();
                            ImGui::Button("Coucou");
                            if (open) {
                                rec(rec, as_nonterminal(node->maps_to));
                                ImGui::TreePop();
                            }
                            ImGui::Unindent();
                        }

                        if (!is_node(node->next))
                            break;

                        node = as_node(node->next);
                    }
                };

                for (auto const & root : input.threads) {
                    print_symbol(print_symbol, root);
                }
            });
        } break;
#endif
    }
}


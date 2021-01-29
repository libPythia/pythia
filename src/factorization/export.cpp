#include "export.hpp"

#include <algorithm>
#include <cassert>
#include <ostream>
#include <set>
#include <tuple>
#include <unordered_map>

auto linearise_grammar(NonTerminal const * root) -> std::vector<Terminal const *> {
    auto res = std::vector<Terminal const *> {};
    if (root == nullptr)
        return res;

    auto buf = std::vector<std::pair<Symbol const *, int>> { { root, 1 } };

    while (buf.size() > 0u) {
        auto const [s, l] = buf.back();
        if (l == 1) {
            buf.pop_back();
        } else {
            --buf.back().second;
        }

        if (is_terminal(s)) {
            res.push_back(as_terminal(s));
        } else if (is_nonterminal(s)) {
            auto n = as_nonterminal(s)->last;
            while (true) {
                buf.push_back({ n->maps_to, n->repeats });
                if (!is_node(n->previous))
                    break;
                n = previous_node(n);
            }
        }
    }

    return res;
}

auto print_trace(NonTerminal const * root, std::ostream & os, terminal_printer const & p) -> void {
    if (root == nullptr)
        return;

    auto buf = std::vector<std::pair<Symbol const *, int>> { { root, 1 } };

    while (buf.size() > 0u) {
        auto const [s, l] = buf.back();
        if (l == 1) {
            buf.pop_back();
        } else {
            --buf.back().second;
        }

        if (is_terminal(s)) {
            p(as_terminal(s), os);
        } else if (is_nonterminal(s)) {
            auto n = as_nonterminal(s)->last;
            while (is_node(n)) {
                buf.push_back({ n->maps_to, n->repeats });
                if (!is_node(n->previous))
                    break;
                n = previous_node(n);
            }
        }
    }
}

auto print_reduced_trace(NonTerminal const * root, std::ostream & os, terminal_printer const & p)
        -> void {
    if (root == nullptr)
        return;

    auto buf = std::vector<std::pair<Symbol const *, int>> { { root, 1 } };

    while (buf.size() > 0u) {
        auto const [s, l] = buf.back();
        buf.pop_back();

        if (s == nullptr) {
            os << ')';
            continue;
        }

        if (l > 1)
            os << l;

        if (is_nonterminal(s) && s != root) {
            os << '(';
            buf.push_back({ nullptr, 0 });
        }

        if (is_terminal(s)) {
            p(as_terminal(s), os);
        } else if (is_nonterminal(s)) {
            auto n = as_nonterminal(s)->last;
            while (true) {
                buf.push_back({ n->maps_to, n->repeats });
                if (!is_node(n->previous))
                    break;
                n = previous_node(n);
            }
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

auto print_grammar(Grammar const & g,
                   NonTerminal const * root,
                   std::ostream & os,
                   terminal_printer const & p) -> void {
    if (root == nullptr) {
        os << "Empty grammar";
    } else {
        auto const nonterminals = build_non_terminal_names(g);
        os << "Grammar root is <" << nonterminals.find(root)->second << '>';

        for (auto const & nonterminal : g.nonterminals.in_use_nonterminals()) {
            os << "\n<" << nonterminals.find(nonterminal)->second << "> ::= ";
            auto n = reinterpret_cast<Base *>(nonterminal->first);
            while (n != nonterminal) {
                auto const node = as_node(n);
                os << ' ';
                if (node->repeats > 1)
                    os << node->repeats;
                if (is_terminal(node->maps_to)) {
                    p(as_terminal(node->maps_to), os);
                } else
                    os << '<' << nonterminals.find(as_nonterminal(node->maps_to))->second << '>';
                n = node->next;
            }
        }
    }
}

auto print_dot_file_begin(std::ostream & os) -> void {
    auto const color = "white";

    // Writing
    os << "digraph g {\n"
          "    rankdir=TD;\n"
          "    bgcolor=\"transparent\";\n"
          "    color=\""
       << color
       << "\";\n"
          "    labeljust=\"c\";\n"
          "    labelloc=\"c\";\n"
          "    fontcolor=\""
       << color
       << "\";\n"
          "    node [color=\""
       << color << "\", fontcolor=\"" << color
       << "\", shape=circle];\n"
          "    edge [color=\""
       << color << "\", fontcolor=\"" << color
       << "\"];\n"
          "\n";
}

auto print_dot_file_grammar(Grammar const & g,
                            NonTerminal const * non_terminal,
                            std::ostream & os,
                            terminal_printer const & p,
                            bool print_input) -> void {
    auto const non_terminals_names = build_non_terminal_names(g);
    auto const non_terminals = g.nonterminals.in_use_nonterminals();
    static auto next_prefix = 0;
    auto const prefix = next_prefix++;

    os << "    subgraph cluster_" << prefix << " {\n";

    if (print_input) {
        os << "    label =\"";
        print_trace(non_terminal, os, p);
        os << "\";\n";
    }

    os << "    /* terminals */\n";

    for (auto const & terminal : g.terminals) {
        auto const ptr = terminal.get();
        os << "\n    \"" << prefix << ptr << "\" [\n";
        os << "        shape=rectangle\n"
              "        label=<\n"
              "          <table border='0' cellborder='0'>\n"
              "            <tr><td port='head'>";
        p(ptr, os);
        os << "</td></tr>\n"
              "          </table>\n"
              "        >\n"
              "    ]\n";
    }

    auto for_each_node = [&](auto non_terminal, auto f) {
        auto node = non_terminal->first;
        while (true) {
            f(node);
            if (!is_node(node->next))
                break;
            node = as_node(node->next);
        }
    };

    os << "\n    /* non_terminals */\n";
    for (auto const & non_terminal : non_terminals) {
        auto const & name = non_terminals_names.at(non_terminal);
        os << "\n    \"" << prefix << non_terminal << "\" [\n";
        os << "        shape=rectangle\n"
              "        label=<\n"
              "          <table border='0' cellborder='0'>\n";
        os << "            <tr>\n"
              "              <td port='head'>"
           << name << " :</td>\n";

        for_each_node(non_terminal, [&](auto node) {
            os << "              <td port='" << node << "'>" << node->repeats << "x</td>\n";
        });

        os << "            </tr>\n"
              "          </table>\n"
              "        >\n"
              "    ]\n";

        for_each_node(non_terminal, [&](auto node) {
            os << "    \"" << prefix << non_terminal << "\":\"" << node << "\" -> \"" << prefix
               << node->maps_to << "\";\n";
        });
    }

    os << "    }\n";
}

auto print_dot_file_end(std::ostream & os) -> void { os << "}"; }

auto print_dot_file(Grammar const & g,
                    NonTerminal const * non_terminal,
                    std::ostream & os,
                    terminal_printer const & p,
                    bool print_input) -> void {
    print_dot_file_begin(os);
    print_dot_file_grammar(g, non_terminal, os, p, print_input);
    print_dot_file_end(os);
}


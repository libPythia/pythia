#include "export.hpp"

#include <algorithm>
#include <cassert>
#include <set>
#include <tuple>

auto linearise_grammar(Grammar const & g) -> std::vector<Terminal const *> {
    auto res = std::vector<Terminal const *> {};
    if (g.root == nullptr)
        return res;

    auto buf = std::vector<std::pair<Symbol const *, int>> { { g.root, 1 } };

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

auto print_trace(Grammar & g, std::ostream & os, terminal_printer const & p) -> void {
    if (g.root == nullptr)
        return;

    auto buf = std::vector<std::pair<Symbol const *, int>> { { g.root, 1 } };

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
                n = previous_node(n);
            }
        }
    }
}

auto print_reduced_trace(Grammar const & g, std::ostream & os, terminal_printer const & p) -> void {
    if (g.root == nullptr)
        return;

    auto buf = std::vector<std::pair<Symbol const *, int>> { { g.root, 1 } };

    while (buf.size() > 0u) {
        auto const [s, l] = buf.back();
        buf.pop_back();

        if (s == nullptr) {
            os << ')';
            continue;
        }

        if (l > 1)
            os << l;

        if (is_nonterminal(s) && s != g.root) {
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

auto print_grammar(Grammar const & g, std::ostream & os, terminal_printer const & p) -> void {
    if (g.root == nullptr) {
        os << "Empty grammar";
    } else {
        auto const nonterminals = [&]() {
            auto tmp = std::unordered_map<NonTerminal const *, char> {};
            auto c = 'A';
            for (auto const & nonterminal : g.nonterminals.in_use_nonterminals()) {
                if (nonterminal == g.root) {
                    tmp.emplace(g.root, 'R');
                } else {
                    if (c == 'R')
                        ++c;
                    tmp.emplace(nonterminal, c++);
                }
            }
            return tmp;
        }();

        os << "Grammar root is <" << nonterminals.find(g.root)->second << '>';

        for (auto const & nonterminal : g.nonterminals.in_use_nonterminals()) {
            os << "\n<" << nonterminals.find(nonterminal)->second << "> ::=";
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

// auto print_dot_file(Grammar &, std::ostream &, terminal_printer const &) -> void {}

// auto writeDotFile(std::ostream & os,
//                   Node const * start,
//                   std::function<std::string(Node const *)> getLabel,
//                   std::function<std::string(LeafId)> getLeaf) -> void {
//     assert(start != nullptr);
//     auto sons = std::vector<std::pair<Node const *, Node const *>> {};
//     auto nexts = std::vector<std::pair<Node const *, Node const *>> {};
//
//     auto buffer = std::vector<Node const *> {};
//     auto nodes = std::set<Node const *> {};
//     buffer.push_back(start);
//
//     while (!buffer.empty()) {
//         auto const node = buffer.back();
//         buffer.pop_back();
//
//         auto ret = nodes.insert(node);
//         if (ret.second) {
//             auto const node_son = node->son();
//             if (node_son != nullptr)
//                 buffer.push_back(node_son);
//
//             if (node->next != nullptr)
//                 buffer.push_back(node->next);
//         }
//     }
//
//     for (auto const node : nodes) {
//         auto const node_son = node->son();
//         if (node_son != nullptr)
//             sons.emplace_back(node, node_son);
//
//         if (node->next != nullptr)
//             nexts.emplace_back(node, node->next);
//     }
//
//     // Writing
//     os << "digraph g {\n";
//     os << "    rankdir=TD;\n";
//     os << "    bgcolor=\"white\";\n";
//     os << "\n";
//
//     os << "    /* Organisation */\n";
//     for (auto const & it : nexts) {
//         os << "    { rank=same; \"" << it.first << "\", \"" << it.second << "\"}\n";
//     }
//
//     os << "\n";
//     os << "    /* Nodes */\n";
//     for (const auto node : nodes) {
//         auto const shape = [&]() {
//             if (node->isLeaf())
//                 return "rectangle";
//             return "ellipse";
//         }();
//         auto const color = [&]() {
//             if (!node->isLeaf() && node->previous == nullptr)
//                 return "red";
//             return "black";
//         }();
//         auto const label = [&]() {
//             if (node->isLeaf()) {
//                 return getLeaf(node->value()) + " (" + getLabel(node) + ")";
//             } else if (node->loop() == 1) { return getLabel(node); } else { return getLabel(node)
//             + " (x " + std::to_string(node->loop()) + ")"; } }(); auto const style = [&] { // if
//             (node->loop() > 1u) return "bold"; return "solid"; }(); os << "    \"" << node << "\"
//             [shape=\"" << shape << "\", color=\"" << color << "\", fillcolor=\"white\", label=\""
//             << label << "\", style=\"" << style << "\"];\n"; } // os << "\n"; os << "    /*
//             Extremum */\n"; // os << "    Start [color=white, fontcolor=white, label=\"\"];\n";
//             os << "    Start -> \"" << start << "\";\n"; os << "\n"; os << "    /* Next */\n";
//             for (auto const & it : nexts) os << "    \"" << it.first << "\" -> \"" << it.second
//             << "\" [color=\"black\", arrowhead=\"normal\"];\n"; os << "\n"; os << "    /* Sons
//             */\n"; for (auto const & it : sons) os << "    \"" << it.first << "\" -> \"" <<
//             it.second << "\" [color=\"blue\", dir=\"both\", arrowhead=\"onormal\", "
//             "arrowtail=\"dot\"];\n";
//     os << "}" << std::endl;
// }


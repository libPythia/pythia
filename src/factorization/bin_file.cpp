#include "bin_file.hpp"

#include <algorithm>
#include <cassert>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <vector>

/* File format
 * nb_terminals:bin_size_t, nb_nonterminals:bin_size_t
 * <terminals>[]
 * <nonterminal>[]
 * root::bin_size_t
 *
 * <terminals>: contains a string describing the symbol prefixed by its size
 * size:bin_size_t
 * data:char[]
 *
 * <nonterminals>:
 * repeats:bin_size_t
 * son:bin_size_t
 *
 */

using bin_size_t = unsigned short;

static auto read_nb(std::istream & is) -> bin_size_t {
    auto res = bin_size_t(0);
    is.read(reinterpret_cast<char *>(&res), sizeof(res));
    return res;
}

static auto write_nb(std::ostream & os, bin_size_t n) -> void {
    os.write(reinterpret_cast<char *>(&n), sizeof(n));
}

auto print_bin_file(Grammar const & g, std::ostream & os, terminal_printer const & printer)
        -> void {
    auto const & terminals = g.terminals;
    auto const & nonterminals = g.nonterminals.in_use_nonterminals();

    std::unordered_map<Symbol const *, bin_size_t> indices;

    write_nb(os, terminals.size());
    write_nb(os, nonterminals.size());

    for (auto const & terminal : terminals) {
        auto ss = std::stringstream {};
        printer(terminal.get(), ss);
        auto const str = ss.str();
        write_nb(os, str.size());
        os.write(str.c_str(), str.size());
        indices.emplace(terminal.get(), indices.size());
    }

    for (auto const & nonterminal : nonterminals) {
        indices.emplace(nonterminal, indices.size());
    }

    assert(indices.size() < std::numeric_limits<bin_size_t>::max());

    for (auto const & nonterminal : nonterminals) {
        auto node = nonterminal->first;
        while (true) {
            assert(node->repeats > 0);
            write_nb(os, node->repeats);
            write_nb(os, indices.at(node->maps_to));
            if (!is_node(node->next))
                break;
            node = as_node(node->next);
        }
        write_nb(os, 0);
    }

    write_nb(os, indices.at(g.root));
}

auto load_bin_file(Grammar & grammar, std::istream & is)
        -> std::unordered_map<Terminal const *, std::string> {
    auto names = std::unordered_map<Terminal const *, std::string> {};

    auto const terminals_count = read_nb(is);
    auto const nonterminals_count = read_nb(is);

    auto symbols = std::vector<Symbol *>(terminals_count + nonterminals_count, nullptr);

    for (auto i = 0u; i < terminals_count; ++i) {
        auto const terminal = new_terminal(grammar, nullptr);
        auto const size = read_nb(is);
        auto str = std::vector<char>(size, 0);
        is.read(&*str.begin(), size);
        names.emplace(terminal, std::string(str.begin(), str.end()));
        symbols[i] = terminal;
    }

    for (auto i = terminals_count; i < symbols.size(); ++i) {
        symbols[i] = grammar.nonterminals.new_nonterminal();
    }

    for (auto i = terminals_count; i < symbols.size(); ++i) {
        auto nonterminal = static_cast<NonTerminal *>(symbols[i]);
        auto previous_node = static_cast<Node *>(nullptr);
        while (true) {
            auto const repeats = read_nb(is);
            if (repeats == 0) {
                assert(previous_node != nullptr);
                previous_node->maps_to->occurences_without_successor.insert(previous_node);
                previous_node->next = nonterminal;
                nonterminal->last = previous_node;
                break;
            }
            auto const node = grammar.nodes.new_node();
            node->maps_to = symbols[read_nb(is)];
            node->repeats = repeats;

            if (previous_node == nullptr) {
                nonterminal->first = node;
                node->previous = nonterminal;
            } else {
                previous_node->maps_to->occurences_with_successor.emplace(node->maps_to,
                                                                          previous_node);
                previous_node->next = node;
                node->previous = previous_node;
            }

            previous_node = node;
        }
    }

    grammar.root = as_nonterminal(symbols.at(read_nb(is)));

    return names;
}


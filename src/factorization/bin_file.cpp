#include "bin_file.hpp"

#include <stdint.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <istream>
#include <limits>
#include <locale>
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

template <typename bin_size_t> static auto read_nb(std::istream & is) -> bin_size_t {
    auto res = bin_size_t(0);
    is.read(reinterpret_cast<char *>(&res), sizeof(res));
    return res;
}

template <typename bin_size_t> static auto write_nb(std::ostream & os, bin_size_t n) -> void {
    os.write(reinterpret_cast<char *>(&n), sizeof(n));
}

template <typename bin_size_t>
static auto constexpr loop_sentinel = std::numeric_limits<bin_size_t>::max() - 1;
template <typename bin_size_t>
static auto constexpr end_sentinel = std::numeric_limits<bin_size_t>::max();

using loop_size_t = std::uint32_t;

template <typename bin_size_t>
static auto print_bin_file_impl(Grammar const & g,
                                std::ostream & os,
                                terminal_printer const & printer) -> void {
    auto const & terminals = g.terminals;
    auto const & nonterminals = g.nonterminals.in_use_nonterminals();

    std::unordered_map<Symbol const *, bin_size_t> indices;

    write_nb<bin_size_t>(os, terminals.size());
    write_nb<bin_size_t>(os, nonterminals.size());

    for (auto const & terminal : terminals) {
        auto ss = std::stringstream {};
        printer(terminal.get(), ss);
        auto const str = ss.str();
        write_nb<bin_size_t>(os, str.size());
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
            if (node->repeats > 1) {
                write_nb<bin_size_t>(os, loop_sentinel<bin_size_t>);
                write_nb<loop_size_t>(os, node->repeats);
            }
            write_nb<bin_size_t>(os, indices.at(node->maps_to));
            if (!is_node(node->next))
                break;
            node = as_node(node->next);
        }
        write_nb<bin_size_t>(os, end_sentinel<bin_size_t>);
    }
}

template <typename bin_size_t>
static auto load_bin_file_impl(Grammar & grammar, std::istream & is) -> void {
    auto const terminals_count = read_nb<bin_size_t>(is);
    auto const nonterminals_count = read_nb<bin_size_t>(is);

    auto symbols = std::vector<Symbol *>(terminals_count + nonterminals_count, nullptr);

    for (auto i = 0u; i < terminals_count; ++i) {
        auto const size = read_nb<bin_size_t>(is);
        auto name = static_cast<char *>(malloc(size + 1));
        is.read(name, size);
        name[size] = '\0';
        auto const terminal = new_terminal(grammar, name);
        symbols[i] = terminal;
    }

    for (auto i = terminals_count; i < symbols.size(); ++i) {
        symbols[i] = grammar.nonterminals.new_nonterminal();
    }

    for (auto i = terminals_count; i < symbols.size(); ++i) {
        auto nonterminal = static_cast<NonTerminal *>(symbols[i]);
        auto previous_node = static_cast<GrammarNode *>(nullptr);
        while (true) {
            auto maps_to = read_nb<bin_size_t>(is);
            if (maps_to == end_sentinel<bin_size_t>) {
                assert(previous_node != nullptr);
                previous_node->maps_to->occurrences_without_successor.insert(previous_node);
                previous_node->next = nonterminal;
                nonterminal->last = previous_node;
                break;
            }

            auto repeats = static_cast<loop_size_t>(1);
            if (maps_to == loop_sentinel<bin_size_t>) {
                repeats = read_nb<loop_size_t>(is);
                maps_to = read_nb<bin_size_t>(is);
            }

            auto const node = grammar.nodes.new_node();
            node->maps_to = symbols[maps_to];
            node->repeats = repeats;

            if (previous_node == nullptr) {
                nonterminal->first = node;
                node->previous = nonterminal;
            } else {
                previous_node->maps_to->occurrences_with_successor.emplace(node->maps_to,
                                                                           previous_node);
                previous_node->next = node;
                node->previous = previous_node;
            }

            previous_node = node;
        }
    }
}

auto print_bin_file(Grammar const & g, std::ostream & os, terminal_printer const & printer)
        -> void {
    auto node_count = g.terminals.size() + g.nonterminals.in_use_nonterminals_count();

    // 'max' and 'max - 1' is reserved so we have one less representable value
    if (node_count <= std::numeric_limits<std::uint8_t>::max() - 2) {
        write_nb<std::uint8_t>(os, 1);
        print_bin_file_impl<std::uint8_t>(g, os, printer);
    } else if (node_count <= std::numeric_limits<std::uint16_t>::max() - 2) {
        write_nb<std::uint8_t>(os, 2);
        print_bin_file_impl<std::uint16_t>(g, os, printer);
    } else if (node_count <= std::numeric_limits<std::uint32_t>::max() - 2) {
        write_nb<std::uint8_t>(os, 3);
        print_bin_file_impl<std::uint32_t>(g, os, printer);
    } else {
        write_nb<std::uint8_t>(os, 4);
        print_bin_file_impl<std::uint64_t>(g, os, printer);
    }
}

auto load_bin_file(Grammar & grammar, std::istream & is) -> void {
    switch (read_nb<std::uint8_t>(is)) {
        case 1: load_bin_file_impl<std::uint8_t>(grammar, is); break;
        case 2: load_bin_file_impl<std::uint16_t>(grammar, is); break;
        case 3: load_bin_file_impl<std::uint32_t>(grammar, is); break;
        case 4: load_bin_file_impl<std::uint64_t>(grammar, is); break;
        default: assert(false);
    }
}

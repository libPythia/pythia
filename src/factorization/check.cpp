#include "check.hpp"

#include <iostream>

auto check_graph_integrity(Grammar const & g) -> bool {
    auto ok = true;
    for (auto const nonterminal : g.nonterminals.in_use_nonterminals()) {
        if (nonterminal->first->previous != nonterminal) {
            std::cerr << "\nFirst node of non-terminal is badly linked";
            ok = false;
        }
        if (nonterminal->last->next != nonterminal) {
            std::cerr << "\nLast node of non-terminal is badly linked";
            ok = false;
        }
        if (auto const last = nonterminal->last;
            last->maps_to->occurences_without_successor.find(last) ==
            last->maps_to->occurences_without_successor.end()) {
            std::cerr << "\nMissing occurence for last symbol of a rule";
            ok = false;
        }

        auto n = nonterminal->first;
        while (!is_last(n)) {
            auto const next = next_node(n);

            if (next->previous != n) {
                std::cerr << "\nTwo successors are badly linked";
                ok = false;
            }
            auto const it = n->maps_to->occurences_with_successor.find(next->maps_to);
            if (it == n->maps_to->occurences_with_successor.end()) {
                std::cerr << "\nA digram is not registered in the first symbol";
                ok = false;
            } else if (it->second != n) {
                std::cerr << "\nA digram is ill-formed";
                ok = false;
            }
            n = next;
        }
        if (n->next != nonterminal) {
            std::cerr << "Last node and first node are not linked to the same non-terminal"
                      << std::endl;
            ok = false;
        }

        // occurences
        for (auto const & [next, occurence] : nonterminal->occurences_with_successor) {
            if (occurence->maps_to != nonterminal) {
                std::cerr << "\nOccurence is registered in wront non-terminal";
                ok = false;
            } else if (is_last(occurence)) {
                std::cerr << "\nOccurence without successor is registered with a successor";
                ok = false;
            } else if (next_node(occurence)->maps_to != next) {
                std::cerr << "\nOccurence with successor is registered with wrong successor";
                ok = false;
            }
        }

        for (auto const & occurence : nonterminal->occurences_without_successor) {
            if (occurence->maps_to != nonterminal) {
                std::cerr << "\nOccurence is registered in wront non-terminal";
                ok = false;
            } else if (!is_last(occurence)) {
                std::cerr << "\nOccurence with successor is registered without a successor";
                ok = false;
            }
        }
    }

    return ok;
}

namespace std {
template <> struct hash<std::pair<Symbol const *, Symbol const *>> {
    inline size_t operator()(const std::pair<Symbol const *, Symbol const *> & v) const {
        return reinterpret_cast<size_t>(v.first) ^ reinterpret_cast<size_t>(v.second);
    }
};

}  // namespace std

static auto is_root(NonTerminal const * nt) -> bool {
    return nt->occurences_with_successor.size() + nt->occurences_without_successor.size() == 0;
}

auto check_grammar_constraints(Grammar const & g) -> bool {
    auto digrams = std::unordered_set<std::pair<Symbol const *, Symbol const *>> {};

    auto ok = true;

    for (auto const nonterminal : g.nonterminals.in_use_nonterminals()) {
        // rules utility
        if (!is_root(nonterminal)) {
            if (nonterminal->first == nonterminal->last) {
                std::cerr << "\nA rule wich is not root contains only one symbol.";
                ok = false;
            }

            auto count = 0u;
            for (auto const & it : nonterminal->occurences_with_successor)
                count += it.second->repeats;
            for (auto const & n : nonterminal->occurences_without_successor)
                count += n->repeats;

            if (count < 2) {
                std::cerr << "\nA rule wich is not root is used only once.";
                ok = false;
            }
        }

        auto n = nonterminal->first;
        while (!is_last(n)) {
            auto const next = next_node(n);

            // No consecutive repetitions
            if (n->maps_to == next->maps_to) {
                std::cerr << "\nSame symbol is used twice consecutively in the grammar.";
                ok = false;
            }
            auto const digram = std::pair { n->maps_to, next->maps_to };

            // digram uniqueness
            auto [it, was_inserted] = digrams.emplace(digram);
            if (!was_inserted) {
                std::cerr << "\nDigram uniqueness is not respected in the grammar.";
                ok = false;
            }

            n = next;
        }
    }

    return ok;
}

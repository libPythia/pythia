#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Base;
struct GrammarNode;
struct Symbol;
struct NonTerminal;
struct Terminal;

// ----------------------------------------------------------

struct Base {
    std::size_t repeats;

  protected:
    Base(std::size_t repeat_value);
};

struct Symbol : public Base {
    std::unordered_map<Symbol *, GrammarNode *> occurrences_with_successor;
    std::unordered_set<GrammarNode *> occurrences_without_successor;

  protected:
    Symbol(std::size_t repeat_value);
};

auto occurrences_count(Symbol const * r) -> std::size_t;

struct NonTerminal final : public Symbol {
    GrammarNode * first = nullptr;
    GrammarNode * last = nullptr;

    NonTerminal();
};

struct Terminal final : public Symbol {
    void * payload = nullptr;

    Terminal();
};

struct GrammarNode final : public Base {
    Base * next = nullptr;      // GrammarNode or nonterminal
    Base * previous = nullptr;  // GrammarNode or nonterminal
    Symbol * maps_to;

    GrammarNode() : Base(1) {}
};

// ----------------------------------------------------------

auto is_nonterminal(Base const * n) -> bool;
auto as_nonterminal(Base * n) -> NonTerminal *;
auto as_nonterminal(Base const * n) -> NonTerminal const *;
auto is_terminal(Base const * n) -> bool;
auto as_terminal(Base * n) -> Terminal *;
auto as_terminal(Base const * n) -> Terminal const *;
auto is_symbol(Base const * n) -> bool;
auto as_symbol(Base * n) -> Symbol *;
auto as_symbol(Base const * n) -> Symbol const *;
auto is_node(Base const * n) -> bool;
auto as_node(Base * n) -> GrammarNode *;
auto as_node(Base const * n) -> GrammarNode const *;
auto is_first(GrammarNode const * n) -> bool;
auto is_last(GrammarNode const * n) -> bool;
auto next_node(GrammarNode * n) -> GrammarNode *;
auto previous_node(GrammarNode * n) -> GrammarNode *;

// ----------------------------------------------------------

class NodeFactory final {
    static auto constexpr chunk_size = 1024;

  public:
    auto new_node() -> GrammarNode *;
    auto release_node(GrammarNode * n) -> void;
    NodeFactory();

  private:
    auto new_chunk() -> void;

  private:
    std::vector<std::vector<GrammarNode>> storage;
    GrammarNode * unused_nodes = nullptr;
};

// ----------------------------------------------------------

class NonTerminalFactory final {
    static auto constexpr chunk_size = 1024;

  public:
    auto new_nonterminal() -> NonTerminal *;
    auto release_nonterminal(NonTerminal * n) -> void;
    auto in_use_nonterminals() const -> std::vector<NonTerminal const *>;
    NonTerminalFactory();

  private:
    auto new_chunk() -> void;

  private:
    std::vector<std::vector<NonTerminal>> storage;
    NonTerminal * unused_nonterminals = nullptr;
};

// ----------------------------------------------------------

struct Grammar {
    std::vector<std::unique_ptr<Terminal>> terminals;
    NodeFactory nodes;
    NonTerminalFactory nonterminals;
};

auto new_terminal(Grammar & g, void * payload) -> Terminal *;
auto insertSymbol(Grammar & g, NonTerminal * nt, Terminal * t) -> NonTerminal *;

// ----------------------------------------------------------


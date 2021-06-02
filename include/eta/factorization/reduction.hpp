#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct GrammarBaseObject;
struct GrammarNode;
struct Symbol;
struct NonTerminal;
struct Terminal;
struct Pattern;

// ----------------------------------------------------------

struct GrammarBaseObject {
    std::size_t repeats;

  protected:
    GrammarBaseObject(std::size_t repeat_value);
};

struct Symbol : public GrammarBaseObject {
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
    Pattern const * pattern = nullptr;  // for prediction

    Terminal();
};

struct GrammarNode final : public GrammarBaseObject {
    GrammarBaseObject * next = nullptr;      // GrammarNode or nonterminal
    GrammarBaseObject * previous = nullptr;  // GrammarNode or nonterminal
    Symbol * maps_to = nullptr;

    GrammarNode() : GrammarBaseObject(1) {}
};

// ----------------------------------------------------------

auto is_nonterminal(GrammarBaseObject const * n) -> bool;
auto as_nonterminal(GrammarBaseObject * n) -> NonTerminal *;
auto as_nonterminal(GrammarBaseObject const * n) -> NonTerminal const *;
auto is_terminal(GrammarBaseObject const * n) -> bool;
auto as_terminal(GrammarBaseObject * n) -> Terminal *;
auto as_terminal(GrammarBaseObject const * n) -> Terminal const *;
auto is_symbol(GrammarBaseObject const * n) -> bool;
auto as_symbol(GrammarBaseObject * n) -> Symbol *;
auto as_symbol(GrammarBaseObject const * n) -> Symbol const *;
auto is_node(GrammarBaseObject const * n) -> bool;
auto as_node(GrammarBaseObject * n) -> GrammarNode *;
auto as_node(GrammarBaseObject const * n) -> GrammarNode const *;
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


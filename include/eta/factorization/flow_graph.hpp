#pragma once

#include <iosfwd>
#include <memory>
#include <unordered_map>
#include <vector>

struct Transition;
struct FlowNode;
struct Terminal;
struct Grammar;

struct Transition final {
    FlowNode * node;
    size_t pop_count;
};

struct FlowNode final {
    FlowNode * son;
    FlowNode * next;
    unsigned long int repeats;
    unsigned long int count;
    Terminal const * first_terminal;
    std::vector<Transition> transitions;
};

class FlowGraph final {
  public:
    auto build_from(Grammar const & g) -> void;
    auto load_from(std::istream & is) -> void;
    auto save_to(std::ostream & os) -> void;

    auto get_entry_point(Terminal const *) const -> FlowNode const *;

    FlowGraph() = default;
    FlowGraph(FlowGraph &&) = default;
    FlowGraph(FlowGraph const &) = delete;
    auto operator=(FlowGraph &&) -> FlowGraph & = default;
    auto operator=(FlowGraph const &) -> FlowGraph & = delete;

  private:
    auto new_node() -> FlowNode *;

  public:
    std::unordered_map<Terminal const *, FlowNode *> entry_points;
    std::vector<std::unique_ptr<FlowNode>> nodes;
};


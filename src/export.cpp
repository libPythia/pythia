#include "export.hpp"

#include <algorithm>
#include <set>
#include <tuple>
#include <vector>

namespace eta {

// -----------------------------------------------------------

auto linearise(Node const * root) -> std::vector<LeafId> {
    auto res = std::vector<LeafId> {};
    if (root == nullptr)
        return res;

    auto buf = std::vector<Node const *> { root };

    while (buf.size() > 0u) {
        auto const node = buf.back();
        buf.pop_back();

        if (node->isLeaf())
            res.push_back(node->value());
        else {
            if (node->next != nullptr)
                buf.push_back(node->next);

            auto const son = node->son();
            if (son != nullptr) {
                auto const loop = node->loop();
                for (auto i = 0u; i < loop; ++i)
                    buf.push_back(son);
            }
        }
    }

    return res;
}

// -----------------------------------------------------------

auto toStr(Node const * root, bool parenthesis, std::function<std::string(LeafId)> getLeaf)
        -> std::string {
    if (root == nullptr)
        return "";

    auto buf = std::vector<Node const *> { root };
    auto res = std::string {};

    while (buf.size() > 0u) {
        auto const node = buf.back();
        buf.pop_back();

        if (parenthesis && node == nullptr) {
            res.push_back(')');
            continue;
        }

        if (node->isLeaf()) {
            assert(node->next == nullptr);
            assert(node->previous == nullptr);
            res.append(getLeaf(node->value()));
        } else {
            if (node->next != nullptr)
                buf.push_back(node->next);

            auto const son = node->son();
            assert(son != nullptr);

            auto const loop = node->loop();
            if (parenthesis) {
                if (loop > 1u)
                    res.append(std::to_string(loop));
                if (!son->isLeaf()) {
                    res.push_back('(');
                    buf.push_back(nullptr);
                }
                buf.push_back(son);
            } else {
                for (auto i = 0u; i < loop; ++i)
                    buf.push_back(son);
            }
        }
    }

    return res;
}

// -----------------------------------------------------------

auto writeDotFile(std::ostream & os,
                  Node const * start,
                  std::function<std::string(Node const *)> getLabel,
                  std::function<std::string(LeafId)> getLeaf) -> void {
    assert(start != nullptr);
    auto sons = std::vector<std::pair<Node const *, Node const *>> {};
    auto nexts = std::vector<std::pair<Node const *, Node const *>> {};

    auto buffer = std::vector<Node const *> {};
    auto nodes = std::set<Node const *> {};
    buffer.push_back(start);

    while (!buffer.empty()) {
        auto const node = buffer.back();
        buffer.pop_back();

        auto ret = nodes.insert(node);
        if (ret.second) {
            auto const node_son = node->son();
            if (node_son != nullptr)
                buffer.push_back(node_son);

            if (node->next != nullptr)
                buffer.push_back(node->next);
        }
    }

    for (auto const node : nodes) {
        auto const node_son = node->son();
        if (node_son != nullptr)
            sons.emplace_back(node, node_son);

        if (node->next != nullptr)
            nexts.emplace_back(node, node->next);
    }

    // Writing
    os << "digraph g {\n";
    os << "    rankdir=TD;\n";
    os << "    bgcolor=\"white\";\n";
    os << "\n";

    os << "    /* Organisation */\n";
    for (auto const & it : nexts) {
        os << "    { rank=same; \"" << it.first << "\", \"" << it.second << "\"}\n";
    }

    os << "\n";
    os << "    /* Nodes */\n";
    for (const auto node : nodes) {
        auto const shape = [&]() {
            if (node->isLeaf())
                return "rectangle";
            return "ellipse";
        }();
        auto const color = [&]() {
            if (!node->isLeaf() && node->previous == nullptr)
                return "red";
            return "black";
        }();
        auto const label = [&]() {
            if (node->isLeaf()) {
                return getLeaf(node->value()) + " (" + getLabel(node) + ")";
            } else if (node->loop() == 1) {
                return getLabel(node);
            } else {
                return getLabel(node) + " (x " + std::to_string(node->loop()) + ")";
            }
        }();
        auto const style = [&] {
            // if (node->loop() > 1u)
            //     return "bold";
            return "solid";
        }();
        os << "    \"" << node << "\" [shape=\"" << shape << "\", color=\"" << color
           << "\", fillcolor=\"white\", label=\"" << label << "\", style=\"" << style << "\"];\n";
    }

    // os << "\n";
    // os << "    /* Extremum */\n";
    // os << "    Start [color=white, fontcolor=white, label=\"\"];\n";
    // os << "    Start -> \"" << start << "\";\n";

    os << "\n";
    os << "    /* Next */\n";
    for (auto const & it : nexts)
        os << "    \"" << it.first << "\" -> \"" << it.second
           << "\" [color=\"black\", arrowhead=\"normal\"];\n";

    os << "\n";
    os << "    /* Sons */\n";
    for (auto const & it : sons)
        os << "    \"" << it.first << "\" -> \"" << it.second
           << "\" [color=\"blue\", dir=\"both\", arrowhead=\"onormal\", arrowtail=\"dot\"];\n";

    os << "}" << std::endl;
}

// -----------------------------------------------------------

}  // namespace eta


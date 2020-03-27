#include "eta_trace/trace_to_string.hpp"

#include <vector>

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

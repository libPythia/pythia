#include "eta_trace/factorization.hpp"

auto searchParentPattern(Node * last, Node * next) -> Node * {
    assert(last != nullptr);
    assert(next != nullptr);
    auto const last_son = last->son();

    if (last_son != nullptr) {
        auto const parent = last_son->parents[next];
        if (parent != nullptr && parent->next != nullptr && parent->next->son() == next) {
            return parent;
        }
    }

    return nullptr;
}

auto getLastPatternNode(Node * node) -> Node * {
    auto last = static_cast<Node *>(nullptr);
    while (node != nullptr) {
        last = node;
        node = node->next;
    }
    return last;
}


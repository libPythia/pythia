#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "eta_trace/allocator.hpp"
#include "eta_trace/factorization.hpp"

using namespace std::string_literals;

template <bool ShowParenthesis, typename Allocator, typename F>
auto getStr(Trace<Allocator> const & trace, F && getValue, NodeId initial_id = NodeId::invalid()) {
    auto buf = std::vector<NodeId> {};
    auto str = std::string {};

    assert(trace.root().isValid());
    buf.push_back(initial_id.isValid() ? initial_id : trace.root());
    assert(buf.back().isValid());

    while (buf.size() > 0u) {
        auto const id = buf.back();
        buf.pop_back();

        if constexpr (ShowParenthesis) {
            if (!id.isValid()) {
                str.push_back(')');
                continue;
            }
        }
        auto const & node = trace[id];

        if (node.next.isValid())
            buf.push_back(node.next);

        auto const son_id = node.son();
        auto const loop_count = node.loop();
        if (son_id.isValid()) {
            if constexpr (ShowParenthesis) {
                if (loop_count > 1)
                    str.append(std::to_string(loop_count));
                buf.push_back(NodeId::invalid());
                str.push_back('(');
                buf.push_back(son_id);
            } else {
                for (auto i = 1u; i < loop_count; ++i) {
                    buf.push_back(son_id);
                }
            }
        }

        if (node.isLeaf()) {
            str.push_back(getValue(node.value()));
        }
    }

    return str;
}

template <typename Allocator, typename F>
auto debug(Trace<Allocator> const & trace, F && getValue) {
    std::cout << "  root : " << trace.root().value() << std::endl;
    for (auto i = 0u; i < trace.nodeCount(); ++i) {
        auto const & node = trace[NodeId(i)];
        std::cout << "  - " << i << std::endl;

        if (node.parents.size() > 0u) {
            std::cout << "    parents: ";
            for (auto [key, id] : node.parents.data()) {
                if (key.isValid())
                    std::cout << id.value() << "->" << key.value() << ", ";
                else
                    std::cout << id.value() << "->∅, ";
            }
            std::cout << std::endl;
        }

        if (node.previous.isValid())
            std::cout << "    previous : " << node.previous.value() << std::endl;
        if (node.next.isValid())
            std::cout << "    next : " << node.next.value() << std::endl;
        if (node.son().isValid())
            std::cout << "    son : " << node.son().value() << std::endl;
        if (node.isLeaf())
            std::cout << "    value  " << node.value().value() << " -> " << getValue(node.value())
                      << std::endl;
    }
}

auto main(int argc, char ** argv) -> int {
    // auto input = "abcabdabcabdabcabdefefefefabd"s;

    assert(argc == 2);
    auto input = std::string(argv[1]);

    auto builder = TraceBuilder<StdAllocator> {};

    auto leafs = std::vector<std::pair<bool, LeafId>> {};

    for (auto c : input) {
        auto const index = static_cast<std::size_t>(c - 'a');
        while (leafs.size() <= index)
            leafs.push_back(std::make_pair(false, LeafId(index)));

        auto & [is_present, id] = leafs[index];
        if (!is_present) {
            id = builder.newLeaf();
            is_present = true;
        }

        builder.insert(id);
    }

    auto getLeaf = [&](LeafId leaf_id) {
        auto index = 0u;
        for (auto const & [exists, id] : leafs) {
            if (id == leaf_id) {
                assert(exists);
                auto const v = static_cast<char>('a' + index);
                std::cerr << "getLeaf(" << leaf_id.value() << ") = " << v << " ";
                return v;
            }
            ++index;
        }
        return 'X';
    };

    std::cout << "Input :  " << input << std::endl;
    std::cout << "Output : " << getStr<false>(builder.trace(), getLeaf) << std::endl;
    std::cout << "Factor : " << getStr<true>(builder.trace(), getLeaf) << std::endl;
    std::cout << "Debug : \n";
    debug(builder.trace(), getLeaf);
}


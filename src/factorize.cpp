#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#define ETA_FACTO_PRINT(args)           \
    if (eta_factorization_print_debugs) \
        std::cerr << ((args));          \
    else {                              \
    }

bool eta_factorization_print_debugs = false;

#include "eta/core/ProgramOptions.hxx"
#include "eta/core/allocator.hpp"
#include "eta/core/rang.hpp"
#include "eta_trace/dot_export.hpp"
#include "eta_trace/factorization.hpp"
#include "eta_trace/trace_to_string.hpp"

using namespace std::string_literals;

auto debug = [](auto & trace, auto && getLeaf) {
    auto nodeId = std::map<Node const *, int> {};
    for (auto i = 0u; i < trace.nodeCount(); ++i)
        nodeId[&trace[i]] = i;

    std::cerr << "root : " << nodeId[trace.root()] << std::endl;

    for (auto i = 0u; i < trace.nodeCount(); ++i) {
        auto node = &trace[i];
        std::cerr << "## " << nodeId[node];
        if (node->loop() > 1u)
            std::cerr << " loop(" << node->loop() << ")";
        if (node == trace.root())
            std::cerr << "  -- root";
        std::cerr << std::endl;

        if (node->next != nullptr)
            std::cerr << "    next : " << nodeId[node->next] << std::endl;
        if (node->previous != nullptr)
            std::cerr << "    previous : " << nodeId[node->previous] << std::endl;
        if (node->parents.size() > 0) {
            std::cerr << "    parents : TODO" << std::endl;
        }
        if (node->isLeaf())
            std::cerr << "    value : " << getLeaf(node->value()) << std::endl;
        else if (node->son() == nullptr)
            std::cerr << "    EMPTY" << std::endl;
        else
            std::cerr << "    son : " << nodeId[node->son()] << std::endl;
    }
};

auto main(int argc, char ** argv) -> int {
    auto parser = po::parser {};

    auto & dump_dot_opt = parser["dot"].abbreviation('d');
    dump_dot_opt.description("Print result under dot format.");

    auto & dump_debug_opt = parser["debug"];
    dump_debug_opt.description("Print debug information instead of factorized trace");

    auto & help_opt = parser["help"].abbreviation('h');
    help_opt.description("Print this help.");

    auto & no_color_opt = parser["no-color"];
    no_color_opt.description("Don't use color in ouput");

    // auto string_opt = parser["string"].abbreviation('s');
    // dump_debug_opt.description("Take each char of input is an event.");

    // auto separator_opt = parser["separator"].abbreviation('S');
    // separator_opt.description("Set the separator to use when parsing input (default is space)");

    auto & print_input_opt = parser["print_input"].abbreviation('i');
    print_input_opt.description("For each output, print the input at the begining of the line");

    auto & input_opt = parser[""].multi();
    input_opt.description("The input as a string");

    if (!parser(argc, argv)) {
        std::cerr << "errors occurred; aborting\n";
        return -1;
    }

    if (help_opt.was_set()) {
        std::cout << parser << std::endl;
        return 0;
    }

    auto const input_from_stdcin = !input_opt.was_set();
    auto const dump_dot_format = dump_dot_opt.was_set();
    eta_factorization_print_debugs = dump_debug_opt.was_set();
    auto const print_input = print_input_opt.was_set();
    auto const no_color = no_color_opt.was_set();

    // TODO incompatibilités

    auto for_each_input = [&](auto && f) {
        auto b = false;
        if (input_from_stdcin) {
            for (std::string input; std::getline(std::cin, input);) {
                if (input != "") {
                    b = true;
                    f(input);
                }
            }
        } else {
            for (auto && input : input_opt) {
                if (input.string != "") {
                    b = true;
                    f(input.string);
                }
            }
        }
        if (!b)
            std::cerr << "No input" << std::endl;
        ;
    };

    for_each_input([&](std::string const & input) {
        auto builder = TraceBuilder<StdAllocator> {};

        auto leafs = std::vector<std::pair<bool, LeafId>> {};

        auto getLeaf = [&leafs](LeafId id) -> std::string {
            auto index = 0u;
            for (auto [is_present, leaf] : leafs) {
                if (is_present && leaf == id) {
                    assert(is_present);
                    return std::string { static_cast<char>('a' + index) };
                }

                ++index;
            }
            return "¤";
        };

        for (auto c : input) {
            auto const index = static_cast<std::size_t>(c - 'a');
            while (leafs.size() <= index)
                leafs.push_back(std::make_pair(false, LeafId(0)));

            auto & [is_present, id] = leafs[index];
            if (!is_present) {
                id = builder.newLeaf();
                is_present = true;
            }
            if (eta_factorization_print_debugs)
                std::cerr << "insert : " << c << " (";
            builder.insert(id);
            if (eta_factorization_print_debugs) {
                std::cerr << ") -> " << toStr(builder.trace().root(), true, getLeaf);
                std::cerr << std::endl;
            }
        }

        auto nodeId = std::map<Node const *, int> {};
        for (auto i = 0u; i < builder.trace().nodeCount(); ++i)
            nodeId[&builder.trace()[i]] = i;

        auto getId = [&nodeId](Node const * ptr) { return std::to_string(nodeId.at(ptr)); };

        if (dump_dot_format)
            writeDotFile(std::cout, builder.trace().root(), getId, getLeaf);
        else {
            if (print_input) {
                if (!no_color)
                    std::cout << rang::fg::red << rang::style::bold;
                std::cout << input << ':';
                if (!no_color)
                    std::cout << rang::style::reset << rang::fg::reset;
            }
            std::cout << toStr(builder.trace().root(), true, getLeaf) << std::endl;
        }
    });
}


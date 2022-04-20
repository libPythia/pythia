#include "prediction.hpp"

#include <cassert>

// -------------------------------------------

auto terminal_ptr_equality(Terminal const * lhs, Terminal const * rhs) -> bool {
    return lhs == rhs;
}

// -------------------------------------------

static auto descend_evaluation(Evaluation & eval) -> void {
    while (true) {
        auto const symbol = eval.back().node->maps_to;
        if (is_terminal(symbol))
            break;
        eval.push_back({ as_nonterminal(symbol)->first, 0 });
    }
}

template <typename F>
static auto next_evaluation_ascend(Symbol const * symbol, F && add_evaluation) -> void {
    auto aux = [&add_evaluation](GrammarNode const * node) {
        if (is_node(node->next)) {
            auto eval = Evaluation { EvaluationNode { as_node(node->next), 0 } };
            descend_evaluation(eval);
            add_evaluation(std::move(eval));
        } else {
            next_evaluation_ascend(as_symbol(node->next), add_evaluation);
        }
    };
    for (auto const & parent : symbol->occurrences_without_successor)
        aux(parent);
    for (auto const & [_, parent] : symbol->occurrences_with_successor)
        aux(parent);
}

// -------------------------------------------

auto init_estimation_from_start(Grammar const & grammar) -> Estimation {
    Estimation res;
    res.emplace_back();

    auto const root = [&]() {
        for (auto const nonterminal : grammar.nonterminals.in_use_nonterminals())
            if (occurrences_count(nonterminal) == 0)
                return nonterminal;
        assert(false);
    }();

    res.back().push_back({ root->first, 0 });
    descend_evaluation(res.back());

    return res;
}

// -------------------------------------------

template <typename F> static auto next_evaluation(Evaluation eval, F && add_evaluation) -> void {
    while (true) {
        auto & eval_node = eval.back();
        auto const next_object = eval_node.node->next;
        if (++eval_node.repeats < eval_node.node->repeats) {  // in loop
            descend_evaluation(eval);
            add_evaluation(std::move(eval));
            break;
        } else if (is_node(next_object)) {  // end loop but has trivial successor
            eval.back().node = as_node(next_object);
            eval.back().repeats = 0;
            descend_evaluation(eval);

            add_evaluation(std::move(eval));
            break;
        } else if (eval.size() == 1) {  // top of eval reached : gain knowledge by exploring parents
            next_evaluation_ascend(as_symbol(next_object), add_evaluation);
            break;
        } else {
            eval.pop_back();  // use existing knowledge in order to find non trivial next
        }
    }
}

auto next_estimation(Estimation estimation,
                     Terminal const * terminal,
                     TerminalEqualityOperator terminals_are_equivalents) -> Estimation {
    auto res = std::vector<Evaluation> {};
    bool loose_knowledge = true;
    for (auto & eval : estimation) {
        auto const eval_terminal = as_terminal(eval.back().node->maps_to);
        if (terminals_are_equivalents(terminal, eval_terminal)) {
            loose_knowledge = false;
            next_evaluation(std::move(eval), [&res](auto e) { res.push_back(std::move(e)); });
        }
    }

    if (loose_knowledge) {
        auto eval = Evaluation {};
        for (auto const parent : terminal->occurrences_without_successor) {
            eval.push_back(EvaluationNode { parent, 1 });
            next_evaluation(std::move(eval), [&res](auto e) { res.push_back(std::move(e)); });
        }
        for (auto const & [_, parent] : terminal->occurrences_with_successor) {
            eval.push_back(EvaluationNode { parent, 1 });
            next_evaluation(std::move(eval), [&res](auto e) { res.push_back(std::move(e)); });
        }
    }

    return res;
}

// -------------------------------------------

auto get_prediction_from_estimation(Estimation & e) -> Prediction { return Prediction { e, 0u }; }

auto get_first_next(Prediction * p) -> bool {
    assert(p->index < p->estimation.size());
    auto last_estimation = std::move(p->estimation[p->index]);
    p->estimation.clear();
    next_evaluation(last_estimation, [p](auto e) { p->estimation.push_back(std::move(e)); });
    return p->estimation.size() > 0;
}

auto get_alternative(Prediction * p) -> bool {
    ++p->index;
    return p->index < p->estimation.size();
}

auto get_terminal(Prediction const & p) -> Terminal const * {
    assert(p.index < p.estimation.size());
    return as_terminal(p.estimation[p.index].back().node->maps_to);
}

// -------------------------------------------

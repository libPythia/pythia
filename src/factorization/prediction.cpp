#include "prediction.hpp"

#include <algorithm>
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
static auto next_evaluation_ascend(NonTerminal const * nt, F && add_evaluation) -> void {
    auto aux = [&add_evaluation](GrammarNode const * node) {
        if (node->repeats > 1) {
            auto eval = Evaluation { EvaluationNode { node, 1 } };
            descend_evaluation(eval);
            add_evaluation(std::move(eval));
        }

        if (is_node(node->next)) {
            auto eval = Evaluation { EvaluationNode { as_node(node->next), 0 } };
            descend_evaluation(eval);
            add_evaluation(std::move(eval));
        } else {
            next_evaluation_ascend(as_nonterminal(node->next), add_evaluation);
        }
    };
    for (auto const & parent : nt->occurrences_without_successor)
        aux(parent);
    for (auto const & [_, parent] : nt->occurrences_with_successor)
        aux(parent);
}

// -------------------------------------------

static auto compute_prevalence(Evaluation const & e) -> size_t {
    auto const get_nonterminal = [](GrammarNode const * node) {
        while (is_node(node->next))
            node = as_node(node->next);
        return as_nonterminal(node->next);
    };

    auto const aux = [&](GrammarNode const * node, auto rec, auto rem) -> size_t {
        auto const nt = get_nonterminal(node);
        auto nt_count = [&]() {
            auto c = 0u;
            for (auto const & parent : nt->occurrences_without_successor)
                c += rec(parent, rec, 0);
            for (auto const & [_, parent] : nt->occurrences_with_successor)
                c += rec(parent, rec, 0);
            return c == 0u ? 1u : c;
        }();
        return (node->repeats - rem) * nt_count;
    };

    return aux(e.front().node, aux, e.front().repeats);
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
            next_evaluation_ascend(as_nonterminal(next_object), add_evaluation);
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

static auto compute_infos(Estimation const & estimation) -> std::vector<Prediction::info_t> {
    auto res = std::vector<Prediction::info_t> {};
    res.reserve(estimation.size());
    for (auto i = 0u; i < estimation.size(); ++i)
        res.push_back({ i, compute_prevalence(estimation[i]) });
    std::sort(res.begin(), res.end(), [](auto const & lhs, auto const & rhs) {
        return lhs.prevalence > rhs.prevalence;
    });
    return res;
}

auto get_prediction_from_estimation(Estimation const & e) -> Prediction {
    return Prediction { e, compute_infos(e) };
}

auto get_first_next(Prediction * p) -> bool {
    assert(p->infos.size() > 0);
    auto last_estimation = std::move(p->estimation[p->infos.back().index]);
    p->estimation.clear();
    p->infos.clear();
    next_evaluation(last_estimation, [p](auto e) {
        assert(e.size() > 0);
        p->estimation.push_back(std::move(e));
    });
    if (p->estimation.size() == 0)
        return false;
    p->infos = compute_infos(p->estimation);
    return true;
}

auto get_alternative(Prediction * p) -> bool {
    p->infos.pop_back();
    return p->infos.size() > 0;
}

auto get_terminal(Prediction const & p) -> Terminal const * {
    assert(p.infos.size() > 0);
    auto const & estimation = p.estimation[p.infos.back().index];
    assert(estimation.size() > 0);
    auto const node = as_terminal(estimation.back().node->maps_to);
    assert(node != nullptr);
    return as_terminal(estimation.back().node->maps_to);
}

// -------------------------------------------

auto get_prevalence(Prediction const & p) -> size_t { return p.infos.back().prevalence; }

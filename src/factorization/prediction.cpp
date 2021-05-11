#include "prediction.hpp"

#include <cassert>
#include <iostream>  // TODO remove

// ----------------------------------------------------------------
// Usefull for both estimation and prediction
// ----------------------------------------------------------------

template <typename T>
static auto add_parent_to_parent_list(T * e, typename T::Parent *& p, GrammarNode const * n) ->
        typename T::Parent * {
    // TODO derecursify ?
    if (p == nullptr) {  // End of list, add new parents
        p = new_parent(e);
        p->next = nullptr;
        p->node = n;
        return p;
    } else if (p->node == n) {  // Parent already exists, reuse it
        return p;
    } else {
        return add_parent_to_parent_list(e, p->next, n);
    }
}

// ----------------------------------------------------------------

template <typename T> static auto explore(T * e, Symbol const * nt) -> void {
    for (auto const [_, n] : nt->occurrences_with_successor)
        descend(e, as_node(n->next));
    for (auto const n : nt->occurrences_without_successor)
        explore(e, as_nonterminal(n->next));
}

// ----------------------------------------------------------------

template <typename T> static auto ascend(T * e, typename T::Parent * p) -> void {
    if (p == nullptr)
        return;

    if (p->count < p->node->repeats) {
        // Repeat
        auto const np = descend(e, p->node);
        if (np != nullptr) {
            np->count = p->count + 1;
            np->parents = p->parents;
            p->parents = nullptr;  // np->parents accidental free
        }
    } else if (is_node(p->node->next)) {
        // Progress horizontally
        auto const np = descend(e, as_node(p->node->next));
        if (np != nullptr) {
            np->parents = p->parents;
            p->parents = nullptr;  // np->parents accidental free
        }
    } else if (p->parents != nullptr) {
        ascend(e, p->parents);
    } else {
        explore(e, as_nonterminal(p->node->next));
    }

    ascend(e, p->next);
}

// ----------------------------------------------------------------
// Estimation
// ----------------------------------------------------------------

auto init_estimation(Estimation * e, Grammar const * g) -> void {
    e->grammar = g;
    e->parents = nullptr;
    e->terminal = nullptr;
    e->next_available_parent = nullptr;
    e->at_begining = true;
}

// ----------------------------------------------------------------

static auto new_parent(Estimation * e) -> Estimation::Parent * {
    if (e->next_available_parent == nullptr) {
        auto const res = new Estimation::Parent {};
        // std::cout << "Create estimation " << res << std::endl;
        return res;
    } else {
        auto const res = e->next_available_parent;
        e->next_available_parent = res->next;
        return res;
    }
}

static auto release_parents(Estimation * e, Estimation::Parent * p) {
    if (p == nullptr)
        return;

    // Recursively release parents and successors
    release_parents(e, p->next);
    release_parents(e, p->parents);

    // Insert in available node list
    p->next = e->next_available_parent;
    p->parents = nullptr;
    e->next_available_parent = p;
}

// ----------------------------------------------------------------

static auto descend(Estimation * e, GrammarNode const * n) -> Estimation::Parent * {
    if (is_terminal(n->maps_to)) {
        auto const t = as_terminal(n->maps_to);
        if (t == e->terminal)
            return add_parent_to_parent_list(e, e->parents, n);
    } else {
        auto const p = descend(e, as_nonterminal(n->maps_to)->first);
        if (p != nullptr)
            return add_parent_to_parent_list(e, p->parents, n);
    }
    return nullptr;
}

// ----------------------------------------------------------------

auto update_estimation(Estimation * e, Terminal const * t) -> void {
    auto const last_terminal = e->terminal;
    e->terminal = t;

    if (t == nullptr) {  // Unexpected event, drop all past knowledge
        release_parents(e, e->parents);
        e->parents = nullptr;
        e->at_begining = false;
    } else {  // Update knowledge
        if (e->at_begining) {
            e->at_begining = false;
            // TODO try to build from roots
            assert(e->parents == nullptr);
        } else if (last_terminal == nullptr) {
            // No prior knownledge :Try to init at the begining of the trace
            assert(e->parents == nullptr);
        } else if (e->parents != nullptr) {  // Try to move from previous estimation
            auto const old_parents = e->parents;
            e->parents = nullptr;
            ascend(e, old_parents);
            release_parents(e, old_parents);
        }

        // If nothing was found, try to restart from last terminal
        if (e->parents == nullptr) {
            for (auto const & node : t->occurrences_without_successor) {
                add_parent_to_parent_list(e, e->parents, node);
            }

            for (auto const & [_, node] : t->occurrences_with_successor) {
                add_parent_to_parent_list(e, e->parents, node);
            }
        }
    }
}

// ----------------------------------------------------------------

static auto destroy_parent(Estimation::Parent * p) -> void {
    if (p == nullptr)
        return;
    destroy_parent(p->parents);
    destroy_parent(p->next);
    // std::cout << "Delete estimation " << p << std::endl;
    delete p;
}

auto deinit_estimation(Estimation * e) -> void {
    destroy_parent(e->parents);
    destroy_parent(e->next_available_parent);
    e->parents = nullptr;
    e->terminal = nullptr;
}

// ----------------------------------------------------------------
// Prediction
// ----------------------------------------------------------------

static auto new_parent(Prediction * p) -> Prediction::Parent * {
    auto const res = [p]() {
        if (p->next_available_parent == nullptr) {
            auto const res = new Prediction::Parent {};
            // std::cout << "Create prediction " << res << std::endl;
            return res;
        } else {
            auto const res = p->next_available_parent;
            p->next_available_parent = res->next;
            return res;
        }
    }();
    res->count = 1;
    res->ref_count = 1;
    return res;
}

static auto release_parents(Prediction * e, Prediction::Parent * p) -> void {
    if (p == nullptr)
        return;

    assert(p->ref_count > 0);
    p->ref_count -= 1;

    if (p->ref_count == 0) {
        // Recursively release parents and successors
        release_parents(e, p->next);
        release_parents(e, p->parents);

        // Insert in available node list
        p->next = e->next_available_parent;
        p->parents = nullptr;
        e->next_available_parent = p;
    }
}

static auto descend(Prediction * e, GrammarNode const * n) -> Prediction::Parent * {
    if (is_terminal(n->maps_to))
        return add_parent_to_parent_list(e, e->parents, n);

    auto const p = descend(e, as_nonterminal(n->maps_to)->first);
    assert(p != nullptr);
    return add_parent_to_parent_list(e, p->parents, n);
}

static auto init_parent(Prediction * p, Estimation::Parent const * e) -> Prediction::Parent * {
    if (e == nullptr)
        return nullptr;
    auto const res = new_parent(p);
    res->node = e->node;
    res->parents = init_parent(p, e->parents);
    res->next = init_parent(p, e->next);
    res->count = e->count;
    return res;
}

auto init_prediction(Prediction * p) -> void {
    p->next_available_parent = nullptr;
    p->parents = nullptr;
}

auto reset_prediction(Prediction * p, Estimation const * e) -> bool {
    release_parents(p, p->parents);
    if (e->at_begining) {
        // init prediction at root of trace
        // TODO prediction before first event is not implemented yet
    } else {
        auto const old_parents = init_parent(p, e->parents);
        ascend(p, old_parents);  // Get next step from estimation
        release_parents(p, old_parents);
    }

    return p->parents != nullptr;
}

auto copy_prediction(Prediction * to, Prediction const * from) -> void {
    release_parents(to, to->parents);
    to->parents = from->parents;
    to->parents->ref_count += 1;
}

auto get_prediction_tree_sibling(Prediction * e) -> bool {
    auto const old_parents = e->parents;
    e->parents = old_parents->next;
    old_parents->next = nullptr;
    release_parents(e, old_parents);
    return e->parents != nullptr;
}

auto get_prediction_tree_child(Prediction * e) -> bool {
    // TODO ????
    auto const old_parents = e->parents;
    e->parents = nullptr;

    assert(old_parents != nullptr);
    auto first_branch = *old_parents;
    first_branch.next = nullptr;
    if (old_parents->parents != nullptr) {
        first_branch.parents->ref_count += 1;
    }
    ascend(e, &first_branch);
    release_parents(e, old_parents);
    return e->parents != nullptr;
}

static auto destroy_parent(Prediction::Parent * p) -> void {
    if (p == nullptr)
        return;
    destroy_parent(p->parents);
    destroy_parent(p->next);
    // std::cout << "Delete prediction " << p << std::endl;
    delete p;
}

auto deinit_prediction(Prediction * p) -> void {
    destroy_parent(p->parents);
    destroy_parent(p->next_available_parent);
    p->parents = nullptr;
    p->next_available_parent = nullptr;
}


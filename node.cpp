#include "node.h"
#include <algorithm>
#include <iostream>
#include <vector>

Position get_other_position(Position const pos){
    return Position(!(int)pos);
}

bool get_is_terminal(Node const& parent, Action const action_taken){
    switch (action_taken)
    {
    case Action::fold:
        return true;
    case Action::raise:
        return false;
    case Action::call:
        if (parent.active_player == Position::ip || parent.parent_bet > 0.1){
            return true;
        }else{
            return false;
        }
    default:
        std::cerr << "is_terminal err" << std::endl;
        exit(1);
    }
}

float get_terminal_pot(Node& parent, Action const action_taken, float const raise_amount){
    switch (action_taken)
    {
    case Action::fold:
        return parent.pot_start_street + (2 * parent.parents_parent_bet); //parents_parent_bet is the amount node 'parent' invested before taking action::fold
    case Action::call:
        return parent.pot_start_street + (2 * parent.parent_bet); //parent_bet is the amount 'parent' was facing before taking action::call
    case Action::raise:
        return 0;//not terminal
    default:
        std::cerr << "get_terminal_pot err" << std::endl;
        exit(1);
    }
}

size_t get_num_bets(Node& parent, Action const action_taken){
    size_t num_bets = parent.num_bets_this_street;
    if (action_taken == Action::raise)
        num_bets++;
    return num_bets;
}

std::string get_child_history(Node& parent, Action const action_taken, float const raise_amount){
    std::string history = parent.history;
    switch (action_taken)
    {
    case Action::fold:
        history += ",F";
        break;
    case Action::call:
        history += ",C";
        break;
    case Action::raise:
        history += ",R";
        history += std::to_string(raise_amount);
        break;
    default:
        std::cerr << "get_child_history err" << std::endl;
        exit(1);
    }
    return history;
}

void add_child(std::vector<Node>& tree, Node& parent, Action const action_taken, float const raise_amount){
    Node child;
    child.history = get_child_history(parent,action_taken,raise_amount);
    child.node_index = tree.size();
    child.dependency_level = parent.dependency_level + 1;
    child.active_player = get_other_position(parent.active_player);
    child.pot_start_street = parent.pot_start_street;
    child.stack_start_street = parent.stack_start_street;
    child.is_terminal = get_is_terminal(parent, action_taken);
    child.is_showdown = (action_taken != Action::fold);
    child.winner_if_nsd_terminal = get_other_position(parent.active_player);
    child.pot_if_terminal = get_terminal_pot(parent,action_taken,raise_amount);
    child.num_bets_this_street = get_num_bets(parent,action_taken);
    child.parent_bet = raise_amount;
    child.parents_parent_bet = parent.parent_bet;
    parent.children_indexes[parent.num_children] = child.node_index;
    parent.num_children++;
    tree.push_back(child);
}

bool can_fold(Node const& node){
    return node.parent_bet > 0.0001;
}

bool can_raise(Node const& node, Subgame const& subgame){
    if (node.parent_bet + 0.0001 > node.stack_start_street ||
        node.num_bets_this_street >= subgame.max_num_bets_per_street)
        return false;
    return true;
}

float get_raise_amount(Node const& node, Subgame const& subgame, float const raise_percent){
    float min_raise = node.parent_bet + (node.parent_bet - node.parents_parent_bet);
    if (min_raise > 1.1 && min_raise < 2.0)
        min_raise = 2.0; //preflop minraise size
    if (min_raise < 1.0)
        min_raise = 1.0;
    float psb = node.pot_start_street + (3 * node.parent_bet);
    float difference = psb - node.parent_bet;
    float amount = node.parent_bet + (difference * raise_percent);
    if (amount < min_raise)
        amount = min_raise;
    if (amount > subgame.all_in_threshold * node.stack_start_street)
        amount = node.stack_start_street;
    if (amount > node.stack_start_street)
        amount = node.stack_start_street;
    return amount;
}

std::vector<float> get_raise_amounts(Node const& node, Subgame const& subgame){
    std::vector<float> raise_percents;
    if (node.num_bets_this_street < subgame.raise_sizes.size()){
        raise_percents = subgame.raise_sizes[node.num_bets_this_street];
    }else{
        raise_percents = subgame.raise_sizes.back();
    }
    std::vector<float> raise_amounts{node.stack_start_street}; //all in is always an option
    for (auto const& perc: raise_percents){
        float amount = get_raise_amount(node,subgame,perc);
        if (std::find(raise_amounts.begin(), raise_amounts.end(), amount) == raise_amounts.end()) //raise amount isnt already in vector
            raise_amounts.push_back(amount);
    }
    return raise_amounts;
}

void add_all_children_r(std::vector<Node>& tree, Node& node, Subgame const& subgame){
    if (!node.is_terminal){
        if (can_fold(node))
            add_child(tree,node,Action::fold,0);
        add_child(tree,node,Action::call,0);
        if (can_raise(node,subgame)){
            std::vector<float> raise_sizes = get_raise_amounts(node,subgame);
            for (auto const& size:raise_sizes){
                add_child(tree,node,Action::raise,size);
            }
        }
        for (size_t i = 0; i < node.num_children; i++)
        {
            std::cout << i;
            std::cout << "subscript: " << node.children_indexes[i] << std::endl;
            add_all_children_r(tree,tree.at(node.children_indexes[i]),subgame);
        }
    }
}

std::vector<Node> build_tree(Subgame const& subgame){
    std::vector<Node> tree;
    Node root;
    root.node_index = 0;
    root.dependency_level = 0;
    root.active_player = Position::oop;
    root.pot_start_street = subgame.pot;
    root.stack_start_street = subgame.stack;
    root.is_terminal = false;
    root.is_showdown = false;
    root.winner_if_nsd_terminal = Position(0);
    root.pot_if_terminal = 0.0;
    root.num_bets_this_street = 0;
    tree.push_back(root);
    add_all_children_r(tree,root,subgame);
    return tree;
}

void print_tree(std::vector<Node> const& tree){
    for (auto const& node:tree)
    {
        std::cout << "Node id: " << node.node_index;
        std::cout << "  Node history: " << node.history << std::endl;
    }
}
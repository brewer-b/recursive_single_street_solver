#ifndef NODE_H
#define NODE_H

#include "subgame.h"
#include <string>
enum class Position{
    oop=0,ip
};

enum class Action{
    fold,call,raise
};

enum class Street{
    preflop=0,flop,turn,river
};

struct Node{
    size_t node_index;
    size_t dependency_level;
    Position active_player;
    float pot_start_street;
    float stack_start_street;
    bool is_terminal;
    bool is_showdown;
    Position winner_if_nsd_terminal;
    float pot_if_terminal;
    size_t num_bets_this_street;
    float parent_bet = 0.0;
    float parents_parent_bet = 0.0;
    size_t num_children = 0;
    size_t children_indexes[MAX_RAISE_SIZES + 2];
    float ** regrets;
    float ** strategy_sum;
    std::string history = "";
};

std::vector<Node> build_tree(Subgame const& subgame);

void print_tree(std::vector<Node> const& tree);

#endif
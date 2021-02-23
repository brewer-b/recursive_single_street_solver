#ifndef TREE_H
#define TREE_H

#include "subgame.h"

#include <string>
#include <vector>

enum class Position {
	oop=0,ip
};
inline Position get_other_position(Position pos) {
	return Position(!(int)pos);
}
enum class Action {
	fold,call,raise
};

struct Payoff {
	bool is_terminal;
	bool is_showdown = 0;
	Position winner_NSD = Position(0);
	double pot = 0.0;
};

struct Node {
	std::string betting_history;
	Payoff payoff;
	std::vector<double> bets_this_street;
	Position active_player;
	std::vector<Node> children;
	std::vector<double> evs = std::vector<double>(1326, 0.0);
	std::vector<double> opp_probs = std::vector<double>(1326, 0.0);
	//hands, actions
	std::vector<std::vector<double>> regrets;
	std::vector<std::vector<double>> current_strategy;
	std::vector<std::vector<double>> cummulative_strategy;
	std::vector<std::vector<double>> avg_strategy;
};

Node build_tree(Tree_Params const& params);

void print_tree(Node const& root);

#endif

#include "tree.h"
#include "subgame.h"

#include <algorithm>
#include <iostream>

void add_child(Tree_Params const& params, Node& parent, Action action_taken, double raise_amount=0.0);

std::string get_child_betting_history(Node& parent, Action action_taken, double raise_amount) {
	std::string history = parent.betting_history;
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

Payoff get_child_payoff(Tree_Params const& params, Node& parent, Action action_taken) {
	Payoff payoff;
	switch (action_taken)
	{
	case Action::fold:
		payoff.is_terminal = true;
		payoff.is_showdown = false;
		payoff.winner_NSD = get_other_position(parent.active_player);
		payoff.pot = params.subgame.pot;
		if (parent.bets_this_street.size() > 1) {
			payoff.pot += (2 * parent.bets_this_street.at(parent.bets_this_street.size()-2)); //2*amount invested before folding
		}
		break;
	case Action::call:
		if (params.subgame.board.size()) { //postflop
			if (parent.active_player == Position::oop && !parent.bets_this_street.size()) {
				payoff.is_terminal = false;
			}
			else {
				payoff.is_terminal = true;
				payoff.is_showdown = true;
				payoff.pot = params.subgame.pot;
				if (parent.bets_this_street.size()) {
					payoff.pot += (2 * parent.bets_this_street.back());//2*bet we are facing before calling
				}
			}
		}
		else {
			if (parent.active_player == Position::ip && (parent.bets_this_street.size() == 2)) {
				payoff.is_terminal = false;
			}
			else {
				payoff.is_terminal = true;
				payoff.is_showdown = true;
				payoff.pot = params.subgame.pot;
				if (parent.bets_this_street.size()) {
					payoff.pot += (2 * parent.bets_this_street.back());//2*bet we are facing before calling
				}
			}
		}
		break;
	case Action::raise:
		payoff.is_terminal = false;
		break;
	default:
		std::cerr << "get_child_payoff err" << std::endl;
		exit(1);
	}
	return payoff;
}

void add_child(Tree_Params const& params, Node& parent, Action action_taken, double raise_amount) {
	Node child;
	child.betting_history = get_child_betting_history(parent, action_taken, raise_amount);
	child.payoff = get_child_payoff(params, parent, action_taken);
	child.bets_this_street = parent.bets_this_street;
	if (raise_amount > 0.1) {
		child.bets_this_street.push_back(raise_amount);
	}
	child.active_player = get_other_position(parent.active_player);
	parent.children.push_back(child);
}

bool can_fold(Tree_Params const& params, Node& parent) {
	if (!params.subgame.board.size() && parent.active_player == Position::oop && parent.bets_this_street.size() == 2) {
		return false;
	}
	else {
		return parent.bets_this_street.size();
	}
}

bool can_raise(Tree_Params const& params, Node& parent) {
	if (parent.bets_this_street.size()) {
		if (parent.bets_this_street.back() + 0.1 >= params.subgame.stack) {
			return false;//facing all in
		}
	}
	if (parent.bets_this_street.size() >= params.betting_params.max_num_bets_per_street) {
		return false;
	}
	return true;
}

double get_raise_amount(Tree_Params const& params, Node const& parent, double const perc) {
	double amount = 1;
	double min_raise = 1;
	if (parent.bets_this_street.size() > 1) {
		min_raise = (parent.bets_this_street.back() - parent.bets_this_street.at(parent.bets_this_street.size() - 2)) + parent.bets_this_street.back();
		if (min_raise < 1)
			min_raise = 1;
	}
	else if (parent.bets_this_street.size() == 1) {
		min_raise = 2 * parent.bets_this_street.back();
	}
	if (!params.subgame.board.size()) {
		if (min_raise < 2.0)
			min_raise = 2.0;
	}
	double bet_facing = 0;
	if (parent.bets_this_street.size()) {
		bet_facing = parent.bets_this_street.back();
	}
	double psb = params.subgame.pot + (3*bet_facing);
	double difference = psb - bet_facing;
	amount = (difference * perc) + bet_facing;
	if (amount < min_raise) {
		amount = min_raise;
	}
	if (amount > params.subgame.stack * params.betting_params.all_in_threshold) {
		amount = params.subgame.stack;
	}
	if (amount > params.subgame.stack) {
		amount = params.subgame.stack;
	}
	return amount;
}

std::vector<double> get_raise_amounts(Tree_Params const& params, Node const& parent) {
	std::vector<double> raise_amounts;
	if (params.betting_params.add_all_in) {
		raise_amounts.push_back(params.subgame.stack);
	}
	std::vector<double> raise_percents;
	size_t raise_num = parent.bets_this_street.size();
	if (!params.subgame.board.size()) {
		raise_num -= 2;
	}
	if (raise_num < params.betting_params.raise_sizes.size()) {
		raise_percents = params.betting_params.raise_sizes.at(raise_num);
	}
	else {
		raise_percents = params.betting_params.raise_sizes.back();
	}
	for (auto const& perc : raise_percents) {
		double amount = get_raise_amount(params, parent, perc);
		if (std::find(raise_amounts.begin(), raise_amounts.end(), amount) == raise_amounts.end()) {
			raise_amounts.push_back(amount);
		}
	}
	return raise_amounts;
}

void add_all_children_r(Tree_Params const& params, Node& parent) {
	if (!parent.payoff.is_terminal) {
		if (can_fold(params, parent)) {
			add_child(params, parent, Action::fold);
		}
		add_child(params, parent, Action::call); //check/call is always an action if it isn't a terminal node
		if (can_raise(params, parent)) {
			std::vector<double> raise_amounts = get_raise_amounts(params, parent);
			for (auto const amount : raise_amounts) {
				add_child(params, parent, Action::raise, amount);
			}
		}
		for (auto& child : parent.children) {
			add_all_children_r(params, child);
		}
	}
}

Node build_tree(Tree_Params const& params) {
	Node root;
	root.betting_history = "root";
	root.payoff.is_terminal = false;
	if (!params.subgame.board.size()) {
		root.bets_this_street.push_back(0.5);
		root.bets_this_street.push_back(1);
	}
	root.active_player = Position::oop;
	if (!params.subgame.board.size())
		root.active_player = Position::ip;
	add_all_children_r(params, root);
	return root;
}

void print_tree(Node const& root) {
	if (root.payoff.is_terminal) {
		std::cout << root.betting_history << "  pot: " << root.payoff.pot << "  is showdown: " << root.payoff.is_showdown <<std::endl;
	}
	for (auto const& child:root.children) {
		print_tree(child);
	}
}
#include "cfr_r.h"

#include "hand.h"
#include "payoff.h"
#include "subgame.h"
#include "tree.h"

#include <iostream>

#define REGRET_EPSILON 0.00001

void cfr_get_current_strategy(Node& node, size_t const num_hands) {
	size_t const num_actions = node.children.size();
	double const default_strategy = 1.0 / num_actions;
	for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
	{
		double positive_regret_sum = 0;
		for (size_t action_index = 0; action_index < num_actions; action_index++)
		{
			double const& regret = node.regrets[hand_index][action_index];
			if (regret > REGRET_EPSILON) {
				positive_regret_sum += regret;
			}
		}
		if (positive_regret_sum > REGRET_EPSILON) {
			for (size_t action_index = 0; action_index < num_actions; action_index++)
			{
				double const& regret = node.regrets[hand_index][action_index];
				if (regret > REGRET_EPSILON) {
					node.current_strategy[hand_index][action_index] = regret / positive_regret_sum;
				}
				else {
					node.current_strategy[hand_index][action_index] = 0.0;
				}
			}
		}else{
			for (size_t action_index = 0; action_index < num_actions; action_index++)
			{
				node.current_strategy[hand_index][action_index] = default_strategy;
			}
		}
		/*
		double played_sum = 0;
		for (size_t action_index = 0; action_index < num_actions; action_index++)
		{
			played_sum += node.current_strategy[hand_index][action_index];
		}
		if (played_sum < 0.9999 || played_sum > 1.0001) {
			std::cout << "current strategy err: " << played_sum << std::endl;
		}
		*/
	}
}

//parent current strategy must already be updated
void update_child_reach_probs(Node const& parent, Node& child, size_t const action_taken_index, size_t const num_hands, Position const iteration_player) {
	memcpy(&child.opp_probs[0], &parent.opp_probs[0], sizeof(parent.opp_probs[0]) * num_hands);
	if (parent.active_player != iteration_player) {
		for (size_t i = 0; i < num_hands; i++)
		{
			child.opp_probs[i] *= parent.current_strategy[i][action_taken_index];
		}
	}
}

//cfr must be called on all children first
void update_evs(Node& node, size_t const num_hands, Position const iteration_player) {
	size_t const& num_children = node.children.size();
	memset(&node.evs[0], 0, num_hands * sizeof(node.evs[0]));
	if (iteration_player == node.active_player) {
		for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
		{
			for (size_t child_index = 0; child_index < num_children; child_index++)
			{
				node.evs[hand_index] += node.current_strategy[hand_index][child_index] * node.children[child_index].evs[hand_index];
			}
		}
	}
	else {
		for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
		{
			for (size_t child_index = 0; child_index < num_children; child_index++)
			{
				node.evs[hand_index] += node.children[child_index].evs[hand_index];
			}
		}
	}
}

void update_regrets(Node& node, size_t const num_hands) {
	size_t const num_actions = node.children.size();
	for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
	{
		for (size_t action_index = 0; action_index < num_actions; action_index++)
		{
			double const& child_ev = node.children[action_index].evs[hand_index];
			double const regret = child_ev - node.evs[hand_index];
			node.regrets[hand_index][action_index] += regret;
		}
	}
}

void update_strategy_sum(Node& node, size_t const num_hands) {
	size_t const num_actions = node.children.size();
	for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
	{
		for (size_t action_index = 0; action_index < num_actions; action_index++)
		{
			node.cummulative_strategy[hand_index][action_index] += node.current_strategy[hand_index][action_index];
		}
	}
}

void cfr_r(Node& node, std::vector<Hand> const& hands, Position const iteration_player) {
	if (node.payoff.is_terminal) {
		if (node.payoff.is_showdown) {
			evalShowdown_2c(node.payoff.pot, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
			//evalShowdown_2c_naive(node.payoff.pot, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
		}
		else {
			double fold_value = node.payoff.pot;
			if (iteration_player != node.payoff.winner_NSD)
				fold_value *= -1;
			evalFold_2c(fold_value, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
			//evalFold_2c_naive(fold_value, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
		}
	}
	else {
		//get current iteration strategy, find ev's of child nodes, get ev of current node, update regrets and strategy sum
		cfr_get_current_strategy(node, hands.size());
		size_t const num_children = node.children.size();
		for (size_t child_index = 0; child_index < num_children; child_index++)
		{
			Node& child = node.children[child_index];
			update_child_reach_probs(node, child, child_index, hands.size(), iteration_player);
			cfr_r(child, hands, iteration_player);
		}
		update_evs(node, hands.size(), iteration_player);
		if (iteration_player == node.active_player) {
			update_regrets(node, hands.size());
			update_strategy_sum(node, hands.size());
		}
	}
}

void init_memory_r(Node& tree, size_t const num_hands) {
	if (!tree.payoff.is_terminal) {
		size_t const num_actions = tree.children.size();
		tree.regrets.resize(num_hands);
		tree.current_strategy.resize(num_hands);
		tree.avg_strategy.resize(num_hands);
		tree.cummulative_strategy.resize(num_hands);
		for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
		{
			tree.regrets[hand_index].resize(num_actions, 0.0);
			tree.current_strategy[hand_index].resize(num_actions, 0.0);
			tree.avg_strategy[hand_index].resize(num_actions, 0.0);
			tree.cummulative_strategy[hand_index].resize(num_actions, 0.0);
		}
		for (auto& child : tree.children) {
			init_memory_r(child, num_hands);
		}
	}
}

void get_average_strategy_r(Node& tree, size_t const num_hands, size_t const num_iterations) {
	if (!tree.payoff.is_terminal) {
		size_t const num_actions = tree.children.size();
		for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
		{
			for (size_t action_index = 0; action_index < num_actions; action_index++)
			{
				tree.avg_strategy[hand_index][action_index] = tree.cummulative_strategy[hand_index][action_index] / num_iterations;
			}
		}
		for (auto& child : tree.children) {
			get_average_strategy_r(child, num_hands, num_iterations);
		}
	}
}

double cfr_br(Node& tree, std::vector<Hand> const& hands, size_t const iteration_number, double const starting_pot);

void run_cfr_r(Node& tree, std::vector<Hand> const& hands, Subgame const& subgame, size_t const num_iterations) {
	init_memory_r(tree, hands.size());
	for (size_t i = 0; i < num_iterations; i++)
	{
		if (i && i % 1000 == 0) {
			double const exploitability = cfr_br(tree, hands, i, subgame.pot);
			std::cout << "exploitability: " << exploitability * 100 << std::endl;
		}
		//std::cout << "cfr iteration :" << i << std::endl;
		memcpy(&tree.opp_probs[0], &subgame.ip_range_sorted[0], hands.size() * sizeof(tree.opp_probs[0]));
		cfr_r(tree, hands, Position::oop);
		memcpy(&tree.opp_probs[0], &subgame.oop_range_sorted[0], hands.size() * sizeof(tree.opp_probs[0]));
		cfr_r(tree, hands, Position::ip);
	}
	get_average_strategy_r(tree, hands.size(), num_iterations);
}

void update_child_reach_probs_br(Node const& parent, Node& child, size_t const action_taken_index, size_t const num_hands, Position const iteration_player) {
	memcpy(&child.opp_probs[0], &parent.opp_probs[0], sizeof(parent.opp_probs[0]) * num_hands);
	if (parent.active_player != iteration_player) {
		for (size_t i = 0; i < num_hands; i++)
		{
			child.opp_probs[i] *= parent.avg_strategy[i][action_taken_index];
		}
	}
}

void update_evs_br(Node& node, size_t const num_hands, Position const iteration_player) {
	size_t const& num_children = node.children.size();
	memset(&node.evs[0], 0, num_hands * sizeof(node.evs[0]));
	if (iteration_player == node.active_player) {
		for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
		{
			double best_ev = 0;
			for (size_t child_index = 0; child_index < num_children; child_index++)
			{
				double const& child_ev = node.children[child_index].evs[hand_index];
				if (child_index == 0 || child_ev > best_ev) {
					best_ev = child_ev;
				}
			}
			node.evs[hand_index] = best_ev;
		}
	}
	else {
		for (size_t hand_index = 0; hand_index < num_hands; hand_index++)
		{
			for (size_t child_index = 0; child_index < num_children; child_index++)
			{
				node.evs[hand_index] += node.children[child_index].evs[hand_index];
			}
		}
	}
}

void cfr_br_r(Node& node, std::vector<Hand> const& hands, Position const iteration_player) {
	if (node.payoff.is_terminal) {
		if (node.payoff.is_showdown) {
			evalShowdown_2c(node.payoff.pot, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
			//evalShowdown_2c_naive(node.payoff.pot, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
		}
		else {
			double fold_value = node.payoff.pot;
			if (iteration_player != node.payoff.winner_NSD)
				fold_value *= -1;
			evalFold_2c(fold_value, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
			//evalFold_2c_naive(fold_value, (int)hands.size(), &hands[0], &node.opp_probs[0], &node.evs[0]);
		}
	}
	else {
		//get current iteration strategy, find ev's of child nodes, get ev of current node, update regrets and strategy sum
		size_t const num_children = node.children.size();
		for (size_t child_index = 0; child_index < num_children; child_index++)
		{
			Node& child = node.children[child_index];
			update_child_reach_probs_br(node, child, child_index, hands.size(), iteration_player);
			cfr_br_r(child, hands, iteration_player);
		}
		update_evs_br(node, hands.size(), iteration_player);
	}
}

double cfr_br(Node& tree, std::vector<Hand> const& hands, size_t const iteration_number, double const starting_pot) {
	get_average_strategy_r(tree, hands.size(), iteration_number);
	double ev_sum = 0;
	cfr_br_r(tree, hands, Position::ip);
	for (size_t i = 0; i < hands.size(); i++)
	{
		ev_sum += tree.evs[i];
	}
	cfr_br_r(tree, hands, Position::oop);
	for (size_t i = 0; i < hands.size(); i++)
	{
		ev_sum += tree.evs[i];
	}
	return ((ev_sum - starting_pot) / starting_pot) / 2;
}
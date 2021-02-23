#include "sanity_tests.h"

#include "card_defines.h"
#include "cfr_r.h"
#include "subgame.h"
#include "tree.h"

#include <iostream>
#include <vector>
#include <string>

void test_print_strategy(Node const& tree,std::string const& node_history, std::vector<Hand> const& sorted_hands) {
	if (!tree.payoff.is_terminal) {
		if (tree.betting_history != node_history) {
			for (auto const& child : tree.children) {
				test_print_strategy(child, node_history, sorted_hands);
			}
		}
		else {
			std::cout << "actions: ";
			for (auto const& child:tree.children)
			{
				std::cout << child.betting_history << "  ";
			}
			std::cout << std::endl;
			for (size_t hand_index = 0; hand_index < sorted_hands.size(); hand_index++)
			{
				sorted_hands[hand_index].print_hand();
				std::cout << ":  ";
				for (size_t action_index = 0; action_index < tree.children.size(); action_index++)
				{
					std::cout << tree.avg_strategy[hand_index][action_index] << "  ";
				}
				std::cout << std::endl;
			}
		}
	}
}

void test_cfr_r() {
	Tree_Params tree_params;
	Subgame subgame;
	subgame.board = std::vector<int>{ c2,s2,c3,s4,sA };
	subgame.pot = 2;
	subgame.stack = 200;
	subgame.spr = subgame.pot / subgame.stack;
	Betting_Params betting_params;
	betting_params.raise_sizes = std::vector<std::vector<double>>{
		{1.0}
	};
	tree_params.betting_params = betting_params;
	tree_params.subgame = subgame;
	Node tree = build_tree(tree_params);
	std::vector<Hand> sorted_hands = get_sorted_hands(subgame);
	for (size_t i = 0; i < sorted_hands.size(); i++)
	{
		//sorted_hands[i].print_hand(false);
		//std::cout << " : " << subgame.ip_range_sorted[i] << "  " << subgame.oop_range_sorted[i] << std::endl;
	}
	run_cfr_r(tree, sorted_hands, subgame, 100000);
	print_tree(tree);
	std::string input;
	while (std::cin >> input) {
		std::cout << "input " << input << std::endl;
		test_print_strategy(tree, input, sorted_hands);
	}
}
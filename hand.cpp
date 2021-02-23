#include "hand.h"

#include "card_defines.h"
#include "hand_evaluator.h"
#include "subgame.h"
#include "tree.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>


void print_card(int card_index, bool newline = false);

std::vector<Hand> get_sorted_hands(Subgame& subgame) {
	std::vector<Hand> hand_vec;
	int index = 0;
	for (int card1 = 1; card1 < 53; card1++)
	{
		for (int card2 = 1; card2 < card1; card2++)
		{
			if (std::find(subgame.board.begin(), subgame.board.end(), card1) != subgame.board.end()||
				std::find(subgame.board.begin(), subgame.board.end(), card2) != subgame.board.end()) {
				index++;
				continue;
			}
			Hand hand;
			hand.holecards[0] = card1;
			hand.holecards[1] = card2;
			hand.raw_index = index;
			hand.initial_weight[(size_t)Position::ip] = subgame.ip_range.at(index);
			hand.initial_weight[(size_t)Position::oop] = subgame.oop_range.at(index);
			hand.strength = LookupHand(&subgame.board[0], &hand.holecards[0]);
			hand_vec.push_back(hand);
			index++;
		}
	}
	std::sort(hand_vec.begin(), hand_vec.end());
	for (size_t i = 0; i < hand_vec.size(); i++)
	{
		subgame.ip_range_sorted[i] = subgame.ip_range.at(hand_vec[i].raw_index);
		subgame.oop_range_sorted[i] = subgame.oop_range.at(hand_vec[i].raw_index);
	}
	return hand_vec;
}
void print_card(int card_index, bool newline) {
	card_index -= c2;
	static std::string ranks = "23456789TJQKA";
	static std::string suits = "cdhs";
	size_t rank_index = card_index / 4;
	size_t suit_index = card_index % 4;
	std::string card_str = { ranks[rank_index],suits[suit_index] };
	std::cout << card_str;
	if (newline)
		std::cout << std::endl;
}


void Hand::print_hand(bool newline) const{
	print_card(holecards[0], false);
	print_card(holecards[1], newline);
}

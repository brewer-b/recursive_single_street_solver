#ifndef HAND_H
#define HAND_H
#include "subgame.h"
#include <vector>

struct Hand {
	int holecards[2];
	int strength;
	int raw_index;
	double initial_weight[2] = { 0 };
	bool operator < (const Hand& compared_hand) const
	{
		return (strength < compared_hand.strength);
	}
	void print_hand(bool newline=false) const;
};

std::vector<Hand> get_sorted_hands(Subgame& subgame);

#endif

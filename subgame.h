#ifndef SUBGAME_H
#define SUBGAME_H

#include <vector>

struct Subgame{
    std::vector<int> board;
    double pot;
    double stack;
    double spr;
    std::vector<double> ip_range = std::vector<double>(1326,1.0/1081);
    std::vector<double> oop_range = std::vector<double>(1326, 1.0/1081);
    std::vector<double> ip_range_sorted = std::vector<double>(1326, 0.0);
    std::vector<double> oop_range_sorted = std::vector<double>(1326, 0.0);
    std::vector<double> ip_evs = std::vector<double>(1326, 0.0);
    std::vector<double> oop_evs = std::vector<double>(1326, 0.0);
};

struct Betting_Params {
    std::vector<std::vector<double>> raise_sizes;
    double all_in_threshold = 1.0;
    size_t max_num_bets_per_street = 1000;
    bool add_all_in = true;
};

struct Tree_Params {
    Subgame subgame;
    Betting_Params betting_params;
};

#endif
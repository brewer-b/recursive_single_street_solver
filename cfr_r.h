#ifndef CFR_H
#define CFR_H

#include "hand.h"
#include "subgame.h"
#include "tree.h"

#include <vector>

void run_cfr_r(Node& tree, std::vector<Hand> const& hands, Subgame const& subgame, size_t const num_iterations);

#endif

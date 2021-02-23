
#include "hand.h"


void evalFold_2c(const double foldValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal)
{
    double sum;
    int hand;
    double sumIncludingCard[52];

    memset(sumIncludingCard, 0, sizeof(sumIncludingCard));
    sum = 0;

    /* One pass over the opponent's hands to build up sums
     * and probabilities for the inclusion / exclusion evaluation */
    for (hand = 0; hand < numHands; hand++) {

        if (oppProbs[hand] > 0.0) {

            sum += oppProbs[hand];
            sumIncludingCard[hands[hand].holecards[0]] += oppProbs[hand];
            sumIncludingCard[hands[hand].holecards[1]] += oppProbs[hand];
        }
    }

    /* One pass over our hands to assign values */
    for (hand = 0; hand < numHands; hand++) {

        retVal[hand] = (double)foldValue
            * (sum
                - sumIncludingCard[hands[hand].holecards[0]]
                - sumIncludingCard[hands[hand].holecards[1]]
                + oppProbs[hand]);
    }
}

void evalShowdown_2c(const double sdValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal)
{
    /* Showdown! */
    double sum;
    int i, j, k;
    double sumIncludingCard[52];

    /* Set up variables */
    sum = 0;
    memset(sumIncludingCard, 0, sizeof(sumIncludingCard));

    /* Consider us losing to everything initially */
    for (k = 0; k < numHands; k++) {

        if (oppProbs[k] > 0.0) {

            sumIncludingCard[hands[k].holecards[0]] -= oppProbs[k];
            sumIncludingCard[hands[k].holecards[1]] -= oppProbs[k];
            sum -= oppProbs[k];
        }
    }

    for (i = 0; i < numHands; ) {

        /* hand i is first in a group of ties; find the last hand in the group */
        for (j = i + 1;
            (j < numHands) && (hands[j].strength == hands[i].strength);
            j++);

        /* Move all tied hands from the lose group to the tie group */
        for (k = i; k < j; k++) {

            sumIncludingCard[hands[k].holecards[0]] += oppProbs[k];
            sumIncludingCard[hands[k].holecards[1]] += oppProbs[k];
            sum += oppProbs[k];
        }

        /* Evaluate all hands in the tie group */
        for (k = i; k < j; ++k) {
            retVal[k] = sdValue
                * (sum
                    - sumIncludingCard[hands[k].holecards[0]]
                    - sumIncludingCard[hands[k].holecards[1]]);
        }

        /* Move this tie group to wins, then move to next tie group */
        for (k = i; k < j; k++) {

            sumIncludingCard[hands[k].holecards[0]] += oppProbs[k];
            sumIncludingCard[hands[k].holecards[1]] += oppProbs[k];
            sum += oppProbs[k];
        }
        i = j;
    }
}

void evalFold_2c_naive(const double foldValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal)
{
    for (int hero_hand = 0; hero_hand < numHands; hero_hand++)
    {
        double sum = 0.0;
        for (int villain_hand = 0; villain_hand < numHands; villain_hand++) {
            if (oppProbs[villain_hand] > 0.0) {
                if (hands[hero_hand].holecards[0] == hands[villain_hand].holecards[0] ||
                    hands[hero_hand].holecards[0] == hands[villain_hand].holecards[1] ||
                    hands[hero_hand].holecards[1] == hands[villain_hand].holecards[0] ||
                    hands[hero_hand].holecards[1] == hands[villain_hand].holecards[1]) continue;
                sum += oppProbs[villain_hand];
            }
        }
        retVal[hero_hand] = (double)foldValue * (sum);
    }
}

void evalShowdown_2c_naive(const double sdValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal)
{
    for (int hero_hand = 0; hero_hand < numHands; hero_hand++)
    {
        double win_prob = 0;
        double loss_prob = 0;
        for (int villain_hand = 0; villain_hand < numHands; villain_hand++) {
            if (oppProbs[villain_hand] > 0.0) {
                if (hands[hero_hand].holecards[0] == hands[villain_hand].holecards[0] ||
                    hands[hero_hand].holecards[0] == hands[villain_hand].holecards[1] ||
                    hands[hero_hand].holecards[1] == hands[villain_hand].holecards[0] ||
                    hands[hero_hand].holecards[1] == hands[villain_hand].holecards[1]) continue;
                if (hands[hero_hand].strength > hands[villain_hand].strength) {
                    win_prob += oppProbs[villain_hand];
                }else if (hands[hero_hand].strength < hands[villain_hand].strength) {
                    loss_prob += oppProbs[villain_hand];
                }
            }
        }
        retVal[hero_hand] = (win_prob - loss_prob) * sdValue;
    }
}

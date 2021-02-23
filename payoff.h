#ifndef PAYOFF_H
#define PAYOFF_H

void evalFold_2c(const double foldValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal);

void evalShowdown_2c(const double sdValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal);

void evalFold_2c_naive(const double foldValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal);

void evalShowdown_2c_naive(const double sdValue,
    const int numHands,
    const Hand* hands,
    const double* oppProbs,
    double* retVal);

#endif

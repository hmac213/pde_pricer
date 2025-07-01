#include "option.h"

// Option base class constructor
Option::Option(double K_, double T_, double r_, double sigma_, double q_)
    : K(K_), T(T_), r(r_), sigma(sigma_), q(q_) {}

// EuropeanCall constructor
EuropeanCall::EuropeanCall(double K_, double T_, double r_, double sigma_, double q_)
    : Option(K_, T_, r_, sigma_, q_) {}

// EuropeanCall methods
double EuropeanCall::payoff(double S) const {
    return std::max(S - K, 0.0);
}

void EuropeanCall::option_price_boundary(double* V_time, const double* S, const double t, int size) const {
    V_time[0] = 0.0;
    V_time[size - 1] = S[size - 1] - K * std::exp(-r * (T - t));
}

// EuropeanPut constructor
EuropeanPut::EuropeanPut(double K_, double T_, double r_, double sigma_, double q_)
    : Option(K_, T_, r_, sigma_, q_) {}

// EuropeanPut methods
double EuropeanPut::payoff(double S) const {
    return std::max(K - S, 0.0);
}

void EuropeanPut::option_price_boundary(double* V_time, const double* S, const double t, int size) const {
    V_time[0] = K * std::exp(-r * (T - t));
    V_time[size - 1] = 0.0;
}
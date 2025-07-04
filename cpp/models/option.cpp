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

// AmericanCall constructor
AmericanCall::AmericanCall(double K_, double T_, double r_, double sigma_, double q_)
    : Option(K_, T_, r_, sigma_, q_) {}

// AmericanCall methods
double AmericanCall::payoff(double S) const {
    return std::max(S - K, 0.0);
}

void AmericanCall::option_price_boundary(double* V_time, const double* S, const double t, int size) const {
    // Lower boundary: option value cannot be negative
    V_time[0] = 0.0;
    
    // Upper boundary: for deep in-the-money, option approaches intrinsic value
    V_time[size - 1] = std::max(S[size - 1] - K, S[size - 1] - K * std::exp(-r * (T - t)));
}

void AmericanCall::early_exercise_condition(double* V_time, const double* S, const double t, int size) const {
    // Apply early exercise condition: V >= intrinsic value
    // American call: V >= max(S - K, 0)
    for (int i = 0; i < size; ++i) {
        double intrinsic_value = payoff(S[i]);
        V_time[i] = std::max(V_time[i], intrinsic_value);
    }
}

// AmericanPut constructor
AmericanPut::AmericanPut(double K_, double T_, double r_, double sigma_, double q_)
    : Option(K_, T_, r_, sigma_, q_) {}

// AmericanPut methods
double AmericanPut::payoff(double S) const {
    return std::max(K - S, 0.0);
}

void AmericanPut::option_price_boundary(double* V_time, const double* S, const double t, int size) const {
    // Lower boundary: deep in-the-money put approaches intrinsic value
    V_time[0] = std::max(K - S[0], K * std::exp(-r * (T - t)) - S[0]);
    
    // Upper boundary: option value cannot be negative
    V_time[size - 1] = 0.0;
}

void AmericanPut::early_exercise_condition(double* V_time, const double* S, const double t, int size) const {
    // Apply early exercise condition: V >= intrinsic value
    // American put: V >= max(K - S, 0)
    for (int i = 0; i < size; ++i) {
        double intrinsic_value = payoff(S[i]);
        V_time[i] = std::max(V_time[i], intrinsic_value);
    }
}
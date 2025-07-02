#ifndef CRANK_NICOLSON_H
#define CRANK_NICOLSON_H

#include <vector>
#include "../models/option.h"

// Tridiagonal solver using Thomas algorithm
std::vector<double> tridiagonal_thomas(
    const std::vector<double>& lower,
    const std::vector<double>& main,
    const std::vector<double>& upper,
    const std::vector<double>& rhs
);

// Main Crank-Nicolson PDE solver
double* solve_crank_nicolson(
    const Option& option,
    const double S_max,
    const double T,
    const int N,
    const int J,
    double* V,
    const double* S,
    const double* t
);

#endif // CRANK_NICOLSON_H 
#include <vector>
#include "../models/option.h"

std::vector<double> tridiagonal_thomas(
    const std::vector<double>& lower,
    const std::vector<double>& main,
    const std::vector<double>& upper,
    const std::vector<double>& rhs
) {
    size_t size = main.size();
    std::vector<double> x(size, 0.0);
    std::vector<double> y(size, 0.0);
    std::vector<double> ans(size, 0.0);

    x[0] = upper[0] / main[0];
    y[0] = rhs[0] / main[0];

    for (size_t i = 1; i < size; ++i) {
        double denom = main[i] - lower[i] * x[i - 1];
        x[i] = upper[i] / denom;
        y[i] = (rhs[i] - lower[i] * y[i - 1]) / denom;
    }

    ans[size - 1] = y[size - 1];

    for (int i = size - 2; i >= 0; --i) {
        ans[i] = y[i] - x[i] * ans[i + 1];
    }

    return ans;
}

double* solve_crank_nicolson(
    const Option& option,
    const double S_max,
    const double T,
    const int N,
    const int J,
    double* V,
    const double* S,
    const double* t
) {
    const double sigma = option.getSigma();
    const double r = option.getR();
    const double dS = S_max / J;
    const double dt = T / N;

    std::vector<double> a(J - 1);
    std::vector<double> b(J - 1);
    std::vector<double> c(J - 1);
    std::vector<double> ML_lower(J - 1);
    std::vector<double> ML_main(J - 1);
    std::vector<double> ML_upper(J - 1);
    std::vector<double> MR_lower(J - 1);
    std::vector<double> MR_main(J - 1);
    std::vector<double> MR_upper(J - 1);
    std::vector<double> rhs(J - 1);
    std::vector<double> thomas_vector(J - 1);

    double sq_sigma = sigma * sigma;
    double sq_S = 0.0;
    double dt_over_sq_dS = (dt / (dS * dS));
    for (int j = 1; j < J; ++j) {
        sq_S = (j * dS) * (j * dS);
        a[j - 1] = 0.5 * sq_sigma * sq_S * dt_over_sq_dS - 0.5 * r * j * dt;
        b[j - 1] = -sq_sigma * sq_S * dt_over_sq_dS - r * dt;
        c[j - 1] = 0.5 * sq_sigma * sq_S * dt_over_sq_dS + 0.5 * r * j * dt;

        ML_lower[j - 1] = -0.5 * a[j - 1];
        ML_main[j - 1] = 1 - 0.5 * b[j - 1];
        ML_upper[j - 1] = -0.5 * c[j - 1];

        MR_lower[j - 1] = 0.5 * a[j - 1];
        MR_main[j - 1] = 1 + 0.5 * b[j - 1];
        MR_upper[j - 1] = 0.5 * c[j - 1];

        rhs[j - 1] = 0.0;
    }

    for (int n = N - 1; n > -1; n--) {
        option.option_price_boundary(V + n * (J + 1), S, t[n], J + 1);
        
        for (int j = 0; j < J - 1; j++) {
            rhs[j] =
                MR_lower[j] * *(V + (n + 1) * (J + 1) + j) +
                MR_main[j] * *(V + (n + 1) * (J + 1) + j + 1) +
                MR_upper[j] * *(V + (n + 1) * (J + 1) + j + 2);
        }

        rhs[0] -= ML_lower[0] * *(V + n * (J + 1));
        rhs[J - 2] -= ML_upper[J - 2] * *(V + n * (J + 1) + J);

        thomas_vector = tridiagonal_thomas(ML_lower, ML_main, ML_upper, rhs);
        for (int j = 1; j < J; j++) {
            *(V + n * (J + 1) + j) = thomas_vector[j - 1];
        }

        // Apply early exercise condition for American options after solving for the time step
        option.early_exercise_condition(V + n * (J + 1), S, t[n], J + 1);
    }

    option.option_price_boundary(V, S, t[0], J + 1);

    return V;
}
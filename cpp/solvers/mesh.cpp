#include "mesh.h"
#include <cstring>  // for memset

MeshData initialize_mesh(
    const Option& option,
    double S_max,
    int N,
    int J
) {
    // Allocate memory for arrays
    int V_size = (N + 1) * (J + 1);
    double* V = new double[V_size];
    double* S = new double[J + 1];
    double* t = new double[N + 1];
    
    // Initialize V to zeros
    memset(V, 0, V_size * sizeof(double));
    
    // Create space grid S: linspace(0, S_max, J + 1)
    for (int j = 0; j <= J; ++j) {
        S[j] = (static_cast<double>(j) / J) * S_max;
    }
    
    // Create time grid t: linspace(0, option.T, N + 1)
    double T = option.getT();
    for (int n = 0; n <= N; ++n) {
        t[n] = (static_cast<double>(n) / N) * T;
    }
    
    // Set terminal payoffs: V[-1, :] = option.payoff(S)
    // In C++: V[N, j] = option.payoff(S[j]) for all j
    for (int j = 0; j <= J; ++j) {
        int terminal_idx = N * (J + 1) + j;
        V[terminal_idx] = option.payoff(S[j]);
    }
    
    // Return MeshData struct
    return MeshData(V, S, t);
} 
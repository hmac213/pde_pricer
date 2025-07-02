import numpy as np
import models.option

def tridiagonal_thomas(lower, main, upper, rhs):
    """
    Perform a Thomas algorithm that maintains the original coefficients
    """
    size = len(rhs)
    x = np.zeros(size,dtype='float64')
    y = np.zeros(size,dtype='float64')
    ans = np.zeros(size,dtype='float64')

    x[0] = upper[0] / main[0]
    y[0] = rhs[0] / main[0]

    for i in range(1, size):
        denom = main[i] - lower[i] * x[i - 1]
        x[i] = upper[i] / denom
        y[i] = (rhs[i] - lower[i] * y[i - 1]) / denom

    ans[size - 1] = y[size - 1]

    for i in reversed(range(size - 1)):
        ans[i] = y[i] - x[i] * ans[i + 1]
    
    return ans

def solve_crank_nicolson(option, S_max, T, N, J, V, S, t):
    """
    We take finite differences of our PDE with f approximating the spatial part of the equation

    (V_new[i] - V_old[i]) / dt = 0.5(f(V_old[i]) + f(V_new[i]))

    We approximate f with a tridiagonal matrix A such that f(v) = -Av

    (I - 0.5dtA)V_new = (I + 0.5dtA)V_old

    We call these new matrices ML and MR
    """
    
    sigma = option.sigma
    r = option.r
    dS = S_max / J
    dt = T / N

    # S values for interior points
    S_interior = S[1:J]

    # lower, main, upper diagonal of A multiplied by dt
    a = 0.5 * sigma**2 * S_interior**2 * (dt / dS**2) - 0.5 * r * S_interior * (dt / dS)
    b = -sigma**2 * S_interior**2 * (dt / dS**2) - r * dt
    c = 0.5 * sigma**2 * S_interior**2 * (dt / dS**2) + 0.5 * r * S_interior * (dt / dS)

    # lower, main, upper diagonal of ML and MR
    ML_lower = -0.5 * a
    ML_main = 1 - 0.5 * b
    ML_upper = -0.5 * c

    MR_lower = 0.5 * a
    MR_main = 1 + 0.5 * b
    MR_upper = 0.5 * c

    for n in reversed(range(N)):
        # Set boundary conditions for the current time step
        option.option_price_boundary(V[n, :], S, t[n])

        rhs = np.zeros(J - 1)
        # handle pure interior calculations
        for idx, j in enumerate(range(1, J)):
            rhs[idx] = (
                MR_lower[idx] * V[n + 1, j - 1] +
                MR_main[idx] * V[n + 1, j] +
                MR_upper[idx] * V[n + 1, j + 1]
            )

        # Adjust rhs for boundary conditions
        rhs[0] -= ML_lower[0] * V[n, 0]
        rhs[-1] -= ML_upper[-1] * V[n, -1]

        V[n, 1:J] = tridiagonal_thomas(ML_lower, ML_main, ML_upper, rhs)
        
        # Apply early exercise condition for American options
        option.early_exercise_condition(V[n, :], S, t[n])

    # set boundaries for zero
    option.option_price_boundary(V[0, :], S, t[0])
    
    # Apply final early exercise condition
    option.early_exercise_condition(V[0, :], S, t[0])

    return V
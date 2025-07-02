import numpy as np

"""
Create a mesh to solve the Black-Stokes equation for different different options.

Parameters:
    option: Option type

    S_max: Theoretical maximum asset price

    N: Number of time steps (Steps of size T / N)
    J: Number of price steps (Steps of size S_max / J)
"""
def initialize_mesh(option, S_max, N, J):
    S = np.linspace(0, S_max, J + 1)
    t = np.linspace(0, option.T, N + 1)

    V = np.zeros((N + 1, J + 1))
    # set terminal payoffs
    V[-1, :] = option.payoff(S)

    return V, S, t

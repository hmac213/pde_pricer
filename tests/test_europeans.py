import numpy as np
import pytest

from models.option import EuropeanCall, EuropeanPut
from solvers.mesh import initialize_mesh
from solvers.crank_nicolson import solve_crank_nicolson

def bs_price_call(S, K, T, r, sigma):
    """Closed-form Black-Scholes European call price."""
    from math import log, sqrt, exp
    from scipy.stats import norm

    if T == 0:
        return max(S - K, 0.0)
    d1 = (log(S / K) + (r + 0.5 * sigma**2)*T) / (sigma * sqrt(T))
    d2 = d1 - sigma * sqrt(T)
    return S * norm.cdf(d1) - K * exp(-r*T) * norm.cdf(d2)

def bs_price_put(S, K, T, r, sigma):
    """Closed-form Black-Scholes European put price."""
    from math import log, sqrt, exp
    from scipy.stats import norm

    if T == 0:
        return max(K - S, 0.0)
    d1 = (log(S / K) + (r + 0.5 * sigma**2)*T) / (sigma * sqrt(T))
    d2 = d1 - sigma * sqrt(T)
    return K * exp(-r*T) * norm.cdf(-d2) - S * norm.cdf(-d1)

@pytest.mark.parametrize("S0,K,T,r,sigma", [
    (50, 50, 1.0, 0.05, 0.2),
    (100, 100, 0.5, 0.01, 0.3),
    (120, 100, 2.0, 0.03, 0.25),
])
def test_european_call_against_bs(S0, K, T, r, sigma):
    # initialize option and mesh
    opt = EuropeanCall(K=K, T=T, r=r, sigma=sigma)
    N, J = 200, 200
    S_max = 3 * K
    V, S_grid, t_grid = initialize_mesh(opt, S_max, N, J)
    price_surface = solve_crank_nicolson(opt, S_max, T, N, J, V, S_grid, t_grid)
    # interpolate at S0
    pn = np.interp(S0, S_grid, price_surface[0])
    analytic = bs_price_call(S0, K, T, r, sigma)
    assert pytest.approx(analytic, rel=1e-3) == pn

@pytest.mark.parametrize("S0,K,T,r,sigma", [
    (50, 50, 1.0, 0.05, 0.2),
    (100, 100, 0.5, 0.01, 0.3),
    (120, 100, 2.0, 0.03, 0.25),
])
def test_european_put_against_bs(S0, K, T, r, sigma):
    # initialize option and mesh
    opt = EuropeanPut(K=K, T=T, r=r, sigma=sigma)
    N, J = 200, 200
    S_max = 3 * K
    V, S_grid, t_grid = initialize_mesh(opt, S_max, N, J)
    price_surface = solve_crank_nicolson(opt, S_max, T, N, J, V, S_grid, t_grid)
    # interpolate at S0
    pn = np.interp(S0, S_grid, price_surface[0])
    analytic = bs_price_put(S0, K, T, r, sigma)
    assert pytest.approx(analytic, rel=1e-3) == pn

def test_call_limits():
    """At S=0, call price → 0; at S→∞, price ~ S - PV(K)."""
    K, T, r, sigma = 100, 1.0, 0.05, 0.2
    opt = EuropeanCall(K, T, r, sigma)
    N, J = 100, 100
    S_max = 10 * K
    V, S_grid, t_grid = initialize_mesh(opt, S_max, N, J)
    price_surface = solve_crank_nicolson(opt, S_max, T, N, J, V, S_grid, t_grid)

    # S = 0
    assert abs(price_surface[0, 0]) < 1e-6

    # S_max boundary ≈ S_max - K*e^{-rT}
    expected = S_max - K * np.exp(-r*T)
    assert pytest.approx(expected, rel=1e-6) == price_surface[0, -1]

def test_put_limits():
    """At S=0, put price → PV(K); at S→∞, put price → 0."""
    K, T, r, sigma = 100, 1.0, 0.05, 0.2
    opt = EuropeanPut(K, T, r, sigma)
    N, J = 100, 100
    S_max = 10 * K
    V, S_grid, t_grid = initialize_mesh(opt, S_max, N, J)
    price_surface = solve_crank_nicolson(opt, S_max, T, N, J, V, S_grid, t_grid)

    # S = 0 boundary
    expected0 = K * np.exp(-r*T)
    assert pytest.approx(expected0, rel=1e-6) == price_surface[0, 0]

    # S_max boundary
    assert abs(price_surface[0, -1]) < 1e-6
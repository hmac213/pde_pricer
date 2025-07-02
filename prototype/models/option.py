import numpy as np

"""
Different supported option classes:
    - European call / put
    - American call / put
"""

class Option:
    def __init__(self, K, T, r, sigma, q = 0.0):
        """
        Parameters:
            K: Strike price
            T: time until maturity (years)
            r: annual risk-free discount rate
            sigma: annual volatility of underlying asset
            q: continuous dividend yield (default 0)
        """
        self.K = K
        self.T = T
        self.r = r
        self.sigma = sigma
        self.q = q

    def payoff(self, S):
        """
        Parameters:
            S: array of asset prices
        """
        raise NotImplementedError
    
    def option_price_boundary(self, V_time, S, t):
        """
        Parameters:
            V_time: array of option prices at current time
            S: corresponding array of underlying asset prices
            t: time elapsed
        """
        raise NotImplementedError
    
    def early_exercise_condition(self, V_time, S, t):
        """
        Apply early exercise condition for American options
        Parameters:
            V_time: array of option prices at current time
            S: corresponding array of underlying asset prices
            t: current time
        """
        # Default: no early exercise (European style)
        pass
    
class EuropeanCall(Option):
    def payoff(self, S):    
        # payoff function: max(S - K, 0)
        return np.maximum(S - self.K, 0.0)
    
    def option_price_boundary(self, V_time, S, t):
        V_time[0] = 0.0
        V_time[-1] = S[-1] - self.K * np.exp(-self.r * (self.T - t))
    
class EuropeanPut(Option):
    def payoff(self, S):
        # payoff function: max(K - S, 0)
        return np.maximum(self.K - S, 0.0)
    
    def option_price_boundary(self, V_time, S, t):
        V_time[0] = self.K * np.exp(-self.r * (self.T - t))
        V_time[-1] = 0.0

class AmericanCall(Option):
    def payoff(self, S):
        # payoff function: max(S - K, 0)
        return np.maximum(S - self.K, 0.0)
    
    def option_price_boundary(self, V_time, S, t):
        # Lower boundary: option value cannot be negative
        V_time[0] = 0.0
        
        # Upper boundary: for deep in-the-money, option approaches intrinsic value
        # But we need to be careful with American options
        V_time[-1] = max(S[-1] - self.K, S[-1] - self.K * np.exp(-self.r * (self.T - t)))
    
    def early_exercise_condition(self, V_time, S, t):
        """
        Apply early exercise condition: V >= intrinsic value
        American call: V >= max(S - K, 0)
        """
        intrinsic_value = self.payoff(S)
        
        # Option value cannot be less than intrinsic value
        V_time[:] = np.maximum(V_time, intrinsic_value)

class AmericanPut(Option):
    def payoff(self, S):
        # payoff function: max(K - S, 0)
        return np.maximum(self.K - S, 0.0)
    
    def option_price_boundary(self, V_time, S, t):
        # Lower boundary: deep in-the-money put approaches intrinsic value
        V_time[0] = max(self.K - S[0], self.K * np.exp(-self.r * (self.T - t)) - S[0])
        
        # Upper boundary: option value cannot be negative
        V_time[-1] = 0.0
    
    def early_exercise_condition(self, V_time, S, t):
        """
        Apply early exercise condition: V >= intrinsic value
        American put: V >= max(K - S, 0)
        """
        intrinsic_value = self.payoff(S)
        
        # Option value cannot be less than intrinsic value
        V_time[:] = np.maximum(V_time, intrinsic_value)
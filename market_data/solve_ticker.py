import yfinance as yf
import pandas as pd
from datetime import datetime, timedelta
from solvers.crank_nicolson import solve_crank_nicolson
from solvers.mesh import initialize_mesh
from market_data.initialize_option import create_option

def get_trading_days(dte):
    today = datetime.now().date()
    
    expiration_date = today + timedelta(days=dte)
    
    business_days = pd.bdate_range(start=today, end=expiration_date)
    
    return max(0, len(business_days) - 1)

def solve_ticker(ticker, option):
    recent_data = yf.Ticker(ticker).history(period='1d', interval='1m')
    latest_price = recent_data['Close'].iloc[-1]
    latest_price = round(latest_price, 2)

    S_max = int(latest_price * 2) # arbitrarily high to avoid potential collisions

    # option.T is now in YEARS, so use it directly
    T_years = option.T
    
    # Calculate trading days for grid sizing only
    trading_days = int(T_years * 252)

    J = int(S_max * 100) # grid point for each cent
    N = max(int(trading_days * 2), 20) # time steps based on trading days

    V, S, t = initialize_mesh(option, S_max, N, J)

    # Pass T_years (not trading days) to the PDE solver
    V_final = solve_crank_nicolson(option, S_max, T_years, N, J, V, S, t)

    # Fix price indexing
    price_index = int(latest_price / S_max * J)
    price_index = min(max(price_index, 0), J-1)

    return V_final[0, price_index]

# TEST
option = create_option('goog', 195, 11, 'american_call')
print(solve_ticker('goog', option))
import yfinance as yf
import pandas as pd
from datetime import datetime, timedelta

# T-Bill symbols can change, but this is a common one for the 13-week (3-month) bill
TREASURY_BILL_TICKER = "^IRX" 

def get_most_recent_rate(ticker: str) -> float:
    """
    Fetches the most recent closing price for a given ticker,
    which for T-bills represents the annualized yield.
    """
    try:
        t_bill = yf.Ticker(ticker)
        # Get historical data for the last 5 days to ensure we get a value
        hist = t_bill.history(period="5d")
        if hist.empty:
            raise ValueError(f"No historical data for {ticker}")
        # The 'Close' price for ^IRX is the yield percentage
        most_recent_yield = hist['Close'].iloc[-1]
        return most_recent_yield / 100.0  # Convert from percentage to decimal
    except Exception as e:
        # Fallback to a default rate if fetching fails
        print(f"Warning: Could not fetch T-bill rate for {ticker}. Error: {e}. Falling back to default 5%.")
        return 0.05

def calculate_risk_free_rate() -> float:
    """
    Calculates the risk-free rate using the 13-week Treasury Bill yield (^IRX).
    This is a standard proxy for the short-term risk-free rate in option pricing.
    """
    return get_most_recent_rate(TREASURY_BILL_TICKER)
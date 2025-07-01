import yfinance as yf
import pandas as pd

def calculate_risk_free_rate(period='1y'):
    """
    Calculate daily risk-free rate using Treasury bill rates
    
    Parameters:
    - period: Time period for rate calculation ('1mo', '3mo', '1y')
    
    Returns:
    - Daily risk-free rate
    """
    
    try:
        # Get 3-month Treasury bill rate (most common proxy for risk-free rate)
        # Using ^IRX (13-week Treasury bill yield)
        treasury = yf.Ticker("^IRX")
        hist = treasury.history(period=period)
        
        if hist.empty:
            # Fallback to a reasonable default
            print("Warning: Could not fetch Treasury rate, using default 4.5%")
            annual_rate = 0.045
        else:
            # Get the most recent rate (already in percentage)
            latest_rate = hist['Close'].iloc[-1]
            annual_rate = latest_rate / 100  # Convert percentage to decimal
        
        return annual_rate
        
    except Exception as e:
        print(f"Error fetching Treasury rate: {e}")
        print("Using default rate of 4.5% annually")
        return 0.045
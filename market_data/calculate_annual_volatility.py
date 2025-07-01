import yfinance as yf
import numpy as np

def calculate_annual_volatility(ticker, period='1y'):
    """
    Calculate annual volatility (sigma) using log returns method
    
    Parameters:
    ticker (str): Stock ticker symbol
    period (str): Time period for historical data ('1y', '6mo', '3mo', '1mo')
    
    Returns:
    float: Annual volatility (sigma)
    """
    
    # Get historical data
    stock = yf.Ticker(ticker)
    hist = stock.history(period=period)
    
    if hist.empty:
        raise ValueError(f"No data found for ticker {ticker}")
    
    # Calculate log returns
    returns = np.log(hist['Close'] / hist['Close'].shift(1)).dropna()
    
    # Calculate daily volatility (standard deviation of returns)
    daily_sigma = returns.std()
    
    # Convert to annual volatility
    annual_sigma = daily_sigma * np.sqrt(252)
    
    return annual_sigma
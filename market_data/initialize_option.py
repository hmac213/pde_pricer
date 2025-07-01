import yfinance as yf
import models.option
from market_data.calculate_annual_volatility import calculate_annual_volatility
from market_data.calculate_risk_free_rate import calculate_daily_risk_free_rate

def create_option(ticker, K, T_days, option_type):
    """
    Create option with proper unit conversions
    
    Parameters:
    ticker: Stock ticker
    K: Strike price
    T_days: Time to expiration in calendar days
    option_type: Option type string
    """
    # Convert calendar days to trading days to years
    import pandas as pd
    from datetime import datetime, timedelta
    
    today = datetime.now().date()
    expiration_date = today + timedelta(days=T_days)
    business_days = pd.bdate_range(start=today, end=expiration_date)
    trading_days = max(0, len(business_days) - 1)
    T_years = trading_days / 252  # Convert to years
    
    # Get annual parameters
    sigma = calculate_annual_volatility(ticker)
    daily_r = calculate_daily_risk_free_rate()
    annual_r = daily_r * 252  # Convert to annual rate

    match option_type:
        case "european_call":
            return models.option.EuropeanCall(K, T_years, annual_r, sigma)
        case "european_put":
            return models.option.EuropeanPut(K, T_years, annual_r, sigma)
        case "american_call":
            return models.option.AmericanCall(K, T_years, annual_r, sigma)
        case "american_put":
            return models.option.AmericanPut(K, T_years, annual_r, sigma)
        case _:
            raise ValueError(f"Invalid option type: {option_type}")
import yfinance as yf
import time
import pandas as pd
from datetime import datetime
from typing import List, Dict, Any
import option_solver_cpp
from market_data.calculate_annual_volatility import calculate_annual_volatility
from market_data.calculate_risk_free_rate import calculate_risk_free_rate

def get_ticker_options(ticker: str) -> List[Dict[str, Any]]:
    """
    Get all available options data for a single ticker, flattened by strike
    
    Returns list of option dictionaries with:
    - strike_price, dte, type, underlying_price, option_price
    """
    try:
        stock = yf.Ticker(ticker)
        
        # Get current stock price
        info = stock.info
        current_price = info.get('currentPrice') or info.get('regularMarketPrice')
        
        if not current_price:
            return []
            
        # Get options expiration dates
        expirations = stock.options
        if not expirations:
            return []
            
        all_options = []
        
        # Get options for each expiration (limit to first 3 for now)
        for exp_date in expirations[:3]:  # Take first 3 expiration dates
            try:
                option_chain = stock.option_chain(exp_date)
                
                # Calculate days to expiration
                exp_datetime = pd.to_datetime(exp_date)
                days_to_exp = (exp_datetime - pd.Timestamp.now()).days
                
                # Process calls
                for _, call_row in option_chain.calls.iterrows():
                    option_data = {
                        'ticker': ticker,
                        'strike_price': call_row['strike'],
                        'dte': days_to_exp,
                        'type': 'call',
                        'underlying_price': current_price,
                        'option_price': call_row['lastPrice'] if pd.notna(call_row['lastPrice']) else call_row['bid']
                    }
                    all_options.append(option_data)
                
                # Process puts
                for _, put_row in option_chain.puts.iterrows():
                    option_data = {
                        'ticker': ticker,
                        'strike_price': put_row['strike'],
                        'dte': days_to_exp,
                        'type': 'put',
                        'underlying_price': current_price,
                        'option_price': put_row['lastPrice'] if pd.notna(put_row['lastPrice']) else put_row['bid']
                    }
                    all_options.append(option_data)
                
            except Exception as e:
                continue
                
        return all_options
        
    except Exception as e:
        return []

def poll_options_data(tickers: List[str], time_interval: int = 30) -> Dict[str, List[Dict[str, Any]]]:
    """
    Poll options data for a list of tickers once
    
    Args:
        tickers: List of ticker symbols
        time_interval: Not used in single poll, kept for compatibility
        
    Returns:
        Dictionary mapping ticker -> list of options
    """
    results = {}
    
    for ticker in tickers:
        options_list = get_ticker_options(ticker)
        results[ticker] = options_list
    
    return results

def create_option_jobs(options_data: Dict[str, List[Dict[str, Any]]]) -> List[option_solver_cpp.OptionJob]:
    """
    Convert polled options data into OptionJob objects
    
    Args:
        options_data: Dictionary mapping ticker -> list of options
        
    Returns:
        List of OptionJob objects ready for processing
    """
    jobs = []
    
    for ticker, options_list in options_data.items():
        if not options_list:
            continue
            
        # Calculate market parameters once per ticker
        try:
            sigma = calculate_annual_volatility(ticker)
            r = calculate_risk_free_rate()
        except Exception as e:
            continue  # Skip this ticker if we can't get market data
        
        # Create OptionJob for each option
        for opt in options_list:
            try:
                # Map option type to C++ expected format
                option_type = "american_call" if opt['type'] == 'call' else "american_put"
                
                job = option_solver_cpp.OptionJob(
                    ticker=opt['ticker'],
                    option_type=option_type,
                    K=opt['strike_price'],
                    T=opt['dte'],
                    current_price=opt['underlying_price'],
                    r=r,
                    sigma=sigma
                )
                jobs.append(job)
                
            except Exception as e:
                continue  # Skip individual options that fail
    
    return jobs

def continuous_poll_and_process(tickers: List[str], callback_function, time_interval: int = 30):
    """
    Continuously poll options data and process jobs with C++ JobQueueProcessor
    
    Args:
        tickers: List of ticker symbols
        callback_function: Function to call with each OptionJobResult
        time_interval: Polling interval in seconds (default 30)
    """
    # Create the processor and job queue
    processor = option_solver_cpp.JobQueueProcessor()
    job_queue = option_solver_cpp.JobQueue()
    
    while True:
        # Poll options data
        options_data = poll_options_data(tickers)
        
        # Convert to OptionJob objects and add to queue
        jobs = create_option_jobs(options_data)
        
        for job in jobs:
            job_queue.add_or_replace_job(job)
        
        if job_queue.size() > 0:  # Only process if we have jobs
            print(f"Processing {job_queue.size()} jobs...")
            # Process jobs in parallel and stream results via callback
            processor.run_batch(job_queue, callback_function)
        
        time.sleep(time_interval)

# Example usage
def example_usage():
    """
    Example of how to use the continuous polling system
    """
    def result_handler(result):
        """Callback function to handle each computed result"""
        print(f"Ticker: {result.ticker}")
        print(f"Option: {result.option_type} K={result.K} T={result.T}")
        print(f"Current Price: ${result.current_price:.2f}")
        print(f"Fair Value: ${result.fair_value:.2f}")
        print("-" * 40)
    
    # Start continuous polling and processing
    tickers = ["AAPL", "GOOGL", "MSFT"]
    
    print("Starting continuous options polling and processing...")
    print("Press Ctrl+C to stop")
    
    try:
        continuous_poll_and_process(tickers, result_handler, time_interval=30)
    except KeyboardInterrupt:
        print("\nStopping...")

if __name__ == "__main__":
    example_usage()
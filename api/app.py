from fastapi import FastAPI
from api.models import TickerRequest, OptionResult
from market_data.options_poller import poll_options_data
from api.redis_cache import RedisCache
from api.lifespan import startup, shutdown
from contextlib import asynccontextmanager
import option_solver_cpp
from api.background_tasks import PollingState, start_polling_loop, stop_polling_loop
from typing import List

# Global state
polling_state = PollingState()
cache = RedisCache()
processor = option_solver_cpp.JobQueueProcessor()
job_queue = option_solver_cpp.JobQueue()

DEFAULT_STARTING_TICKERS = ['AAPL', 'GOOG', 'CELH', 'MSFT']

def restart_polling_loop():
    """Stop the current polling loop and start a new one with updated tickers."""
    stop_polling_loop(polling_state)
    
    updated_tickers = cache.get_cached_tickers()
    if updated_tickers:
        start_polling_loop(
            polling_state, 
            updated_tickers, 
            cache.cache_option_job_result,
            processor,
            job_queue
        )
    else:
        print("No tickers to poll.")

# handle startup and shutdown
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Initial startup logic
    startup(cache, DEFAULT_STARTING_TICKERS)
    restart_polling_loop()  # Start the first polling loop
    
    yield
    
    # Shutdown logic
    print("Shutting down...")
    stop_polling_loop(polling_state)
    shutdown(cache)
    print("Shutdown complete.")

app = FastAPI(lifespan=lifespan)

@app.get("/")
def read_root():
    return {"message": "Hello, World!"} 
    
@app.post('/add_ticker')
def add_ticker(request: TickerRequest):
    cache.cache_ticker(request.ticker)
    restart_polling_loop()
    return {"message": f"Ticker {request.ticker} added and polling loop restarted"}

@app.delete('/delete_ticker')
def delete_ticker(request: TickerRequest):
    cache.delete_ticker(request.ticker)
    restart_polling_loop()
    return {"message": f"Ticker {request.ticker} deleted and polling loop restarted"}

@app.get('/get_cached_tickers')
def get_cached_tickers():
    return cache.get_cached_tickers()

@app.get('/get_cached_options_for_ticker', response_model=List[OptionResult])
def get_cached_options_for_ticker(ticker: str):
    return cache.get_cached_options_for_ticker(ticker)
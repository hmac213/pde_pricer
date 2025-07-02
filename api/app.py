from fastapi import FastAPI
from typing import List
from models import TickerRequest
from market_data.options_poller import poll_options_data
from redis_cache import RedisCache

app = FastAPI()
cache = RedisCache()

@app.get("/")
def read_root():
    return {"message": "Hello, World!"} 
    
@app.post('/add_ticker')
def add_ticker(request: TickerRequest):
    cache.cache_ticker(request.ticker)
    return {"message": f"Ticker {request.ticker} added to cache"}

@app.get('/get_cached_tickers')
def get_cached_tickers():
    return cache.get_cached_tickers()

@app.get('/get_cached_options_for_ticker')
def get_cached_options_for_ticker(ticker: str):
    return cache.get_cached_options_for_ticker(ticker)
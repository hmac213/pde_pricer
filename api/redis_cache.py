import redis
import json
from option_solver_cpp import OptionJobResult

class RedisCache:
    def __init__(self, host='redis', port=6379):
        self.redis_client = redis.Redis(host=host, port=port, decode_responses=True)

    def cache_ticker(self, ticker: str):
        self.redis_client.rpush('active_tickers', ticker)

    def get_cached_tickers(self):
        return self.redis_client.lrange('active_tickers', 0, -1)

    def cache_option(self, ticker: str, option: OptionJobResult):
        key = f'option:{ticker}:{option.option_type}:{option.K}:{option.T}'
        self.redis_client.delete(key) # clear previous value
        self.redis_client.hset(key, mapping={
            'ticker': option.ticker,
            'option_type': option.option_type,
            'K': option.K,
            'T': option.T,
            'current_price': option.current_price,
            'current_option_price': option.current_option_price,
            'fair_value': option.fair_value
        })

    def get_cached_options_for_ticker(self, ticker: str):
        keys = self.redis_client.keys(f'option:{ticker}:*')
        return [self.redis_client.get(key) for key in keys]
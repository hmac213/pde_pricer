import redis
from option_solver_cpp import OptionJobResult

class RedisCache:
    def __init__(self, host='redis', port=6379):
        self.redis_client = redis.Redis(host=host, port=port, decode_responses=True)

    def cache_ticker(self, ticker: str):
        self.redis_client.rpush('active_tickers', ticker)

    def get_cached_tickers(self):
        return self.redis_client.lrange('active_tickers', 0, -1)

    def cache_tickers(self, tickers: list[str]):
        if self.redis_client.exists('active_tickers'):
            self.redis_client.delete('active_tickers')
        self.redis_client.rpush('active_tickers', *tickers)

    def cache_option_job_result(self, option: OptionJobResult):
        ticker = option.ticker
        option_key = f'option:{ticker}:{option.option_type}:{option.K}:{option.T}'
        ticker_set_key = f'options_for:{ticker}'

        # Use a pipeline for atomic operations
        with self.redis_client.pipeline() as pipe:
            pipe.delete(option_key) # clear previous value to ensure freshness
            pipe.hset(option_key, mapping={
                'ticker': option.ticker,
                'option_type': option.option_type,
                'K': option.K,
                'T': option.T,
                'current_price': option.current_price,
                'current_option_price': option.current_option_price,
                'fair_value': option.fair_value
            })
            pipe.sadd(ticker_set_key, option_key)
            pipe.execute()

    def delete_ticker(self, ticker: str):
        ticker_set_key = f'options_for:{ticker}'
        option_keys = self.redis_client.smembers(ticker_set_key)
        
        # Use a pipeline to delete all related keys efficiently
        with self.redis_client.pipeline() as pipe:
            if option_keys:
                pipe.delete(*option_keys)
            pipe.delete(ticker_set_key)
            pipe.lrem('active_tickers', 0, ticker)
            pipe.execute()

    def get_cached_options_for_ticker(self, ticker: str):
        ticker_set_key = f'options_for:{ticker}'
        option_keys = self.redis_client.smembers(ticker_set_key)
        if not option_keys:
            return []
        
        # Use a pipeline to fetch all options at once
        with self.redis_client.pipeline() as pipe:
            for key in option_keys:
                pipe.hgetall(key)
            return pipe.execute()
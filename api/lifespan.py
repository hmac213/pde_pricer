from api.redis_cache import RedisCache
from typing import List

def startup(cache: RedisCache, starting_tickers: List[str]):
    cache.cache_tickers(starting_tickers)

def shutdown(cache: RedisCache):
    cache.redis_client.close()
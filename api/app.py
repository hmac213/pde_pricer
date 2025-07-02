from fastapi import FastAPI
from typing import List
from models import OptionRequest, OptionResponse
from market_data.initialize_option import create_option
from market_data.solve_ticker import solve_ticker

app = FastAPI()

@app.get("/")
def read_root():
    return {"message": "Hello, World!"}

# @app.get('/option_prices')
# def get_option_prices(request: List[OptionRequest]):
#     option = create_option(request.ticker, request.K, request.T_days, request.option_type)

# IMPLEMENT GRABBING FROM CACHE    
    


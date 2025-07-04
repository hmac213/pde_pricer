from pydantic import BaseModel

class TickerRequest(BaseModel):
    ticker: str

class OptionResult(BaseModel):
    ticker: str
    option_type: str
    K: float
    T: float
    current_price: float
    current_option_price: float
    fair_value: float
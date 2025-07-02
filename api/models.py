from pydantic import BaseModel

class OptionRequest(BaseModel):
    ticker: str
    K: float
    T_days: int
    option_type: str

class OptionResponse(BaseModel):
    option_price: float
    fair_value: float
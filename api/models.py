from pydantic import BaseModel

class TickerRequest(BaseModel):
    ticker: str
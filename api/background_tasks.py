import threading
from typing import List, Callable
from market_data.options_poller import continuous_poll_and_process
import option_solver_cpp

class PollingState:
    def __init__(self):
        self.stop_event = threading.Event()
        self.polling_thread = None

def start_polling_loop(
    state: PollingState, 
    tickers: List[str], 
    callback: Callable, 
    processor: option_solver_cpp.JobQueueProcessor, 
    job_queue: option_solver_cpp.JobQueue
):
    state.stop_event.clear()
    state.polling_thread = threading.Thread(
        target=continuous_poll_and_process,
        args=(tickers, callback, state.stop_event, processor, job_queue)
    )
    state.polling_thread.start()

def stop_polling_loop(state: PollingState):
    if state.polling_thread and state.polling_thread.is_alive():
        state.stop_event.set()
        state.polling_thread.join()
        print("Polling loop stopped.") 
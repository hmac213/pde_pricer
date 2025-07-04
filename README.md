# PDE-Based Options Pricer

This project implements a high-performance options pricing engine using a C++ backend accelerated with `pybind11`, exposed via a Python FastAPI REST API. It continuously polls for real-world options data, calculates their fair value using a Partial Differential Equation (PDE) approach, and caches the results in Redis.

## Table of Contents
- [Mathematical Motivation](#mathematical-motivation)
  - [The Black-Scholes-Merton Model](#the-black-scholes-merton-model)
  - [Limitations and the PDE Approach](#limitations-and-the-pde-approach)
- [Algorithmic Implementation](#algorithmic-implementation)
  - [Finite Difference Methods](#finite-difference-methods)
  - [The Crank-Nicolson Scheme](#the-crank-nicolson-scheme)
- [System Design and Architecture](#system-design-and-architecture)
  - [1. C++ Core](#1-c-core)
  - [2. Pybind11 Wrapper](#2-pybind11-wrapper)
  - [3. Python Application Layer](#3-python-application-layer)
- [Lifecycle Management and Concurrency](#lifecycle-management-and-concurrency)
  - [Application Startup](#application-startup)
  - [Continuous Polling](#continuous-polling)
  - [Dynamic Updates](#dynamic-updates)
  - [Graceful Shutdown](#graceful-shutdown)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Running the Application](#running-the-application)
- [API Endpoints](#api-endpoints)

---

## Mathematical Motivation

### The Black-Scholes-Merton Model

The Black-Scholes-Merton (BSM) model is a cornerstone of modern financial theory. It provides a closed-form solution for pricing European-style options, which can only be exercised at expiration. The model is derived from a set of assumptions about the market (e.g., constant volatility, risk-free rate, no dividends, and log-normal distribution of stock returns).

The BSM model's elegance lies in its ability to construct a risk-neutral portfolio of the underlying asset and a risk-free bond that perfectly replicates the option's payoff. The price of the option is then the cost of this replicating portfolio.

### Limitations and the PDE Approach

The closed-form BSM solution breaks down for more complex, "exotic" options, particularly **American-style options**. These options can be exercised at any time before or at expiration, introducing an "early exercise" feature that cannot be modeled with a simple formula.

To solve this, we can re-frame the BSM model as a **Partial Differential Equation**. The BSM PDE describes how the value of an option, \(V\), changes with respect to time, \(t\), and the price of the underlying asset, \(S\):

\[ \frac{\partial V}{\partial t} + rS \frac{\partial V}{\partial S} + \frac{1}{2}\sigma^2 S^2 \frac{\partial^2 V}{\partial S^2} - rV = 0 \]

This PDE provides a more flexible framework. While it's more computationally intensive, it allows us to solve for the price of options with features that violate the strict assumptions of the closed-form solution, such as the early-exercise feature of American options.

## Algorithmic Implementation

### Finite Difference Methods

Since the BSM PDE rarely has an analytical solution for complex options, we turn to numerical methods. This project uses a **finite difference method** to approximate the solution.

The core idea is to discretize the continuous domain of the PDE into a finite grid.
-   The **asset price** (\(S\)) is discretized into a set number of steps from 0 to a maximum value (\(S_{max}\)).
-   **Time** (\(t\)) is discretized into steps from the present (t=0) to the option's expiration (\(t=T\)).

By replacing the partial derivatives in the PDE with finite difference approximations (e.g., using forward, backward, or central differences), we transform the PDE into a large system of linear equations that can be solved numerically.

### The Crank-Nicolson Scheme

This project implements the **Crank-Nicolson scheme**, a highly regarded finite difference method. It is an implicit method that averages the finite difference approximations at the current time step (\(t_i\)) and the next time step (\(t_{i+1}\)).

This averaging gives the method excellent **stability properties** and achieves **second-order accuracy** in both time and space, making it more reliable than simpler explicit methods, which can diverge if the grid steps are not chosen carefully. The system of equations is solved by stepping backward in time from the known option payoff at expiration to find its value at the present time.

## System Design and Architecture

The application is designed as a multi-layered system to separate concerns and maximize performance.

![System Architecture Diagram](https://i.imgur.com/your-diagram-image.png) <!-- Replace with a real diagram if you have one -->

### 1. C++ Core

-   **`solvers/`**: Contains the core numerical logic. `crank_nicolson.cpp` holds the implementation of the finite difference scheme.
-   **`models/`**: Defines the data structures for options (e.g., `AmericanCall`, `EuropeanPut`).
-   **`job_queue.h/cpp`**: Manages the multi-threaded processing of option pricing jobs. The `JobQueueProcessor` is the heart of the C++ backend. It takes a queue of pricing jobs, spins up a pool of C++ worker threads (`std::thread`) equal to the number of available hardware cores, and processes the jobs in parallel. To avoid deadlocks and race conditions with Python, it collects all results internally and returns them in a single batch.

### 2. Pybind11 Wrapper

-   **`bindings.cpp`**: This file is the bridge between C++ and Python. It uses `pybind11` to expose the C++ classes (`OptionJob`, `JobQueueProcessor`, etc.) and functions to the Python interpreter as a native module (`option_solver_cpp`). This allows Python code to instantiate and interact with high-performance C++ objects directly.

### 3. Python Application Layer

-   **`market_data/`**: Contains Python scripts responsible for fetching real-world market data. `options_poller.py` uses the `yfinance` library to get option chains for a list of stock tickers.
-   **`api/`**: A `FastAPI` web application that provides the REST interface.
    -   **`redis_cache.py`**: A dedicated class for managing the connection and data flow with the Redis cache. It uses efficient Redis data structures (Hashes for option data, Sets for indexing) to ensure fast lookups and avoid slow `KEYS` operations.
    -   **`lifespan.py`**: Manages application startup and shutdown events, such as connecting to and closing the Redis connection.
    -   **`app.py`**: The main FastAPI application file. It defines the API endpoints (`/add_ticker`, `/get_cached_options_for_ticker`, etc.) and orchestrates the application's lifecycle.

## Lifecycle Management and Concurrency

The application runs a continuous background task, which requires careful management of threads and state.

-   **`api/background_tasks.py`**: This module contains the logic for managing the background polling loop.

### Application Startup

1.  Uvicorn starts the FastAPI application.
2.  The `lifespan` manager is triggered. It connects to Redis and caches an initial list of default tickers.
3.  A dedicated Python background thread (`threading.Thread`) is launched to run the `continuous_poll_and_process` function. This prevents the long-running task from blocking the main web server thread.

### Continuous Polling

The background thread runs an infinite loop that:
1.  Polls `yfinance` for options data for the active tickers.
2.  Creates `OptionJob` objects for each option.
3.  Passes the job queue to the C++ `JobQueueProcessor`.
4.  The C++ backend processes all jobs in parallel.
5.  As results are returned, a callback function (`cache_option_job_result`) is invoked to save each result to the Redis cache.

### Dynamic Updates

-   When a user hits the `/add_ticker` or `/delete_ticker` endpoints, the list of active tickers in Redis is updated.
-   A `restart_polling_loop` function is then called. It uses a `threading.Event` to signal the current background thread to stop gracefully. Once the old thread has exited, a new one is started with the updated list of tickers.

### Graceful Shutdown

-   When the application is shut down (e.g., via Ctrl+C), the `lifespan` manager's shutdown phase is triggered.
-   It signals the background thread to stop and waits for it to exit cleanly using `thread.join()`.
-   Finally, it closes the connection to the Redis server.

## Getting Started

### Prerequisites

-   Docker
-   Docker Compose

### Running the Application

1.  **Clone the repository:**
    ```sh
    git clone <your-repo-url>
    cd pde_pricer
    ```

2.  **Build and run the services:**
    This command will build the C++ extension, install Python dependencies, and start the FastAPI application and the Redis container.
    ```sh
    docker-compose up --build
    ```

3.  The API will be available at `http://localhost:8000`.

## API Endpoints

-   `GET /`: Root endpoint, returns a welcome message.
-   `POST /add_ticker`: Adds a new stock ticker to the polling list.
    -   **Body**: `{"ticker": "SPY"}`
-   `DELETE /delete_ticker`: Removes a ticker from the polling list.
    -   **Body**: `{"ticker": "SPY"}`
-   `GET /get_cached_tickers`: Returns a list of all tickers currently being polled.
-   `GET /get_cached_options_for_ticker`: Returns a list of all cached option data for a given ticker.
    -   **Query Parameter**: `?ticker=AAPL` 
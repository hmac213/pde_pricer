#include "job_queue.h"
#include "solvers/mesh.h"
#include "solvers/crank_nicolson.h"
#include <algorithm>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// OptionJob implementation

OptionJob::OptionJob(
    std::string ticker,
    std::string option_type,
    double K,
    double T,
    double current_price,
    double current_option_price,
    double r,
    double sigma,
    double q
) : ticker(ticker), option_type(option_type), K(K), T(T), 
    current_price(current_price), current_option_price(current_option_price), r(r), sigma(sigma), q(q) { 
    option = create_option();
    S_max = calculate_S_max();
    J = calculate_J();
    N = calculate_N();
}

double OptionJob::calculate_S_max() const {
    if (option_type == "call" || option_type == "american_call" || option_type == "european_call") {
        // For calls: use 4x max of current price or strike, with volatility buffer
        double base_max = std::max(current_price, K) * 4.0;
        double vol_adjustment = current_price * sigma * std::sqrt(T / 365.0) * 3.0; // 3 standard deviations
        return base_max + vol_adjustment;
    } else {
        // For puts: theoretical max is strike, but add buffer for safety
        return K * 1.5;
    }
}

int OptionJob::calculate_J() const {
    return static_cast<int>(S_max * 100); // One grid point per cent
}

int OptionJob::calculate_N() const {
    // T is now in years, so multiply by 365 to get days
    int days = static_cast<int>(T * 365);
    int steps_per_day = 10;
    int min_steps = 200;
    int calculated_steps = days * steps_per_day;
    return std::max(calculated_steps, min_steps);
}

// Copy constructor implementation
OptionJob::OptionJob(const OptionJob& other)
    : ticker(other.ticker), option_type(other.option_type), K(other.K), T(other.T),
      current_price(other.current_price), current_option_price(other.current_option_price), 
      r(other.r), sigma(other.sigma), q(other.q),
      S_max(other.S_max), J(other.J), N(other.N) {
    // Create a new copy of the option
    option = create_option();
}

// Assignment operator implementation
OptionJob& OptionJob::operator=(const OptionJob& other) {
    if (this != &other) {
        // Clean up existing option
        delete option;
        
        // Copy all members
        ticker = other.ticker;
        option_type = other.option_type;
        K = other.K;
        T = other.T;
        current_price = other.current_price;
        current_option_price = other.current_option_price;
        r = other.r;
        sigma = other.sigma;
        q = other.q;
        
        S_max = other.S_max;
        J = other.J;
        N = other.N;
        
        // Create a new copy of the option
        option = create_option();
    }
    return *this;
}

Option* OptionJob::create_option() {
    if (option_type == "european_call") {
        return new EuropeanCall(K, T, r, sigma, q);
    } else if (option_type == "european_put") {
        return new EuropeanPut(K, T, r, sigma, q);
    } else if (option_type == "american_call") {
        return new AmericanCall(K, T, r, sigma, q);
    } else if (option_type == "american_put") {
        return new AmericanPut(K, T, r, sigma, q);
    } else {
        throw std::invalid_argument("Invalid option type");
    }
}

// JobQueue implementation

void JobQueue::add_or_replace_job(const OptionJob& job) {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    if (seen_keys.find(job) == seen_keys.end()) {
        job_queue.push(job);
        seen_keys.insert(job);
    }
}

void JobQueue::remove_job(const OptionJob& job) {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    seen_keys.erase(job);
}

OptionJobResult JobQueue::run_job(const OptionJob& job) {
    Option* option = job.get_option();
    MeshData mesh = initialize_mesh(*option, job.get_S_max(), job.get_N(), job.get_J());
    double* grid_values = solve_crank_nicolson(
        *option,
        job.get_S_max(),
        job.get_T(),
        job.get_N(),
        job.get_J(),
        mesh.V,
        mesh.S,
        mesh.t
    );
    
    // Calculate fair price from resulting grid
    double dS = job.get_S_max() / job.get_J();
    int space_index = std::min((int)(job.get_current_price() / dS), job.get_J());
    int time_index = 0; // Present value (t = 0)
    
    // Grid is indexed as V[time_step * (J+1) + space_step]
    double fair_price = *(grid_values + time_index * (job.get_J() + 1) + space_index);

    // create result object
    OptionJobResult result(job.get_ticker(), job.get_option_type(), job.get_K(), job.get_T(), job.get_current_price(), job.get_current_option_price(), fair_price);
    return result;
}


std::vector<OptionJob> JobQueue::get_all_jobs() {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    std::vector<OptionJob> jobs;
    
    while (!job_queue.empty()) {
        jobs.push_back(job_queue.front());
        job_queue.pop();
    }
    
    // Clear the seen_keys set since we're processing all jobs
    seen_keys.clear();
    
    return jobs;
}

size_t JobQueue::size() const {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    return job_queue.size();
}

OptionJob JobQueue::front() const {
    std::lock_guard<std::mutex> lock(job_queue_mutex);
    return job_queue.front();
}

// JobQueueProcessor implementation

void JobQueueProcessor::run_batch(JobQueue& queue, std::function<void(OptionJobResult)> callback) {
    // Get all jobs from the queue
    std::vector<OptionJob> jobs = queue.get_all_jobs();
    
    if (jobs.empty()) return;
    
    std::vector<std::thread> threads;
    
    // Use minimum of available threads and number of jobs
    size_t actual_threads = std::min(num_threads, jobs.size());
    threads.reserve(actual_threads);

    size_t jobs_per_thread = jobs.size() / actual_threads;
    size_t remainder = jobs.size() % actual_threads;

    for (size_t thread = 0; thread < actual_threads; ++thread) {
        size_t start = thread * jobs_per_thread + std::min(thread, remainder);
        size_t end = start + jobs_per_thread + (thread < remainder ? 1 : 0);

        threads.emplace_back([this, &jobs, &callback, start, end]() {
            for (size_t i = start; i < end; ++i) {
                OptionJobResult result = job_queue.run_job(jobs[i]);
                
                {
                    py::gil_scoped_acquire gil;
                    std::lock_guard<std::mutex> lock(callback_mutex);
                    callback(result);
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

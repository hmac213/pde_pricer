#include "job_queue.h"
#include "solvers/mesh.h"
#include "solvers/crank_nicolson.h"
#include <algorithm>

// OptionJob implementation

OptionJob::OptionJob(
    std::string ticker,
    std::string option_type,
    double K,
    int T,
    double current_price,
    double r,
    double sigma,
    double q = 0.0
) : ticker(ticker), option_type(option_type), K(K), T(T), 
    current_price(current_price), r(r), sigma(sigma), q(q) { 
    create_option();
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
    int steps_per_day = 10;
    int min_steps = 200;
    int calculated_steps = T * steps_per_day;
    return std::max(calculated_steps, min_steps);
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

double* JobQueue::run_job(const OptionJob& job) {
    Option* option = job.get_option();
    MeshData mesh = initialize_mesh(*option, job.get_S_max(), job.get_J(), job.get_N());
    double* result = solve_crank_nicolson(
        *option,
        job.get_S_max(),
        job.T,
        job.get_N(),
        job.get_J(),
        mesh.V,
        mesh.S,
        mesh.t
    );
    return result;
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

std::vector<OptionJobResult> JobQueueProcessor::run_batch(const std::vector<OptionJob>& jobs, const int batch_size) {
    std::vector<OptionJobResult> results;
    threads.reserve(batch_size);

    for (size_t i = 0; i < batch_size; ++i) {
        threads.emplace_back([this, &jobs, &results]() {
            OptionJob job = job_queue.front();
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return results;
}

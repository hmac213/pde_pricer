#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <set>
#include <memory>
#include <tuple>
#include <thread>
#include <functional>
#include "models/option.h"

// Option jobs
class OptionJob {
public:
    OptionJob(
        std::string ticker,
        std::string option_type,
        double K,
        double T,
        double current_price,
        double current_option_price,
        double r,
        double sigma,
        double q
    );
    
    // Copy constructor
    OptionJob(const OptionJob& other);
    
    // Assignment operator
    OptionJob& operator=(const OptionJob& other);
    
    // Destructor
    ~OptionJob() { delete option; }
    
    // Getter methods for all members
    inline const std::string& get_ticker() const { return ticker; }
    inline const std::string& get_option_type() const { return option_type; }
    inline double get_K() const { return K; }
    inline double get_T() const { return T; }
    inline double get_current_price() const { return current_price; }
    inline double get_current_option_price() const { return current_option_price; }
    inline double get_r() const { return r; }
    inline double get_sigma() const { return sigma; }
    inline double get_q() const { return q; }
    
    // Getters for computed private members
    inline double get_S_max() const { return S_max; }
    inline int get_J() const { return J; }
    inline int get_N() const { return N; }
    inline Option* get_option() const { return option; }

    // Comparison operators for uniqueness
    bool operator<(const OptionJob& other) const {
        return
            std::tie(ticker, option_type, K, T) < 
            std::tie(other.ticker, other.option_type, other.K, other.T);
    }

private:
    // Core option parameters
    std::string ticker;
    std::string option_type;
    double K;
    double T;
    double current_price;
    double current_option_price;
    double r; // r and sigma are calculated from python market
    double sigma;
    double q;
    
    // Computed members since they're implementation details
    double S_max;
    int J;
    int N;
    Option* option;

    // private helper methods for initialization
    double calculate_S_max() const;
    int calculate_J() const;
    int calculate_N() const;
    Option* create_option();
};

struct OptionJobResult {
    std::string ticker;
    std::string option_type;
    double K;
    double T;
    double current_price;
    double current_option_price;
    double fair_value; // computed by the PDE solver

    OptionJobResult(
        std::string ticker,
        std::string option_type,
        double K,
        double T,
        double current_price,
        double current_option_price,
        double fair_value
    ) : ticker(ticker), option_type(option_type), K(K), T(T), current_price(current_price), current_option_price(current_option_price), fair_value(fair_value) {}
};

class JobQueue {
private:
    std::queue<OptionJob> job_queue;
    std::set<OptionJob> seen_keys;
    mutable std::mutex job_queue_mutex;
public:
    void add_or_replace_job(const OptionJob& job);
    void remove_job(const OptionJob& job);
    OptionJobResult run_job(const OptionJob& job);
    std::vector<OptionJob> get_all_jobs();  // Get all jobs and clear the queue
    size_t size() const;
    OptionJob front() const;
};

class JobQueueProcessor {
private:
    JobQueue job_queue;
    size_t num_threads;
    std::mutex callback_mutex;
public:
    JobQueueProcessor() : num_threads(std::thread::hardware_concurrency()) { if (num_threads == 0) { num_threads = 1; } }

    // override default constructor
    JobQueueProcessor(size_t num_threads) : num_threads(num_threads) { if (num_threads == 0) { num_threads = 1; } }

    // Process jobs from queue and stream results via callback
    void run_batch(JobQueue& queue, std::function<void(OptionJobResult)> callback);
};

#endif // JOB_QUEUE_H 
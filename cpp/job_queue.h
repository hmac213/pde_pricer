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
#include "models/option.h"

// Option jobs
class OptionJob {
public:
    // public for convenience (immutable after construction)
    const std::string ticker;
    const std::string option_type;
    const double K;
    const int T;
    const double current_price;
    const double r;
    const double sigma;
    const double q;

    OptionJob(
        std::string ticker,
        std::string option_type,
        double K,
        int T,
        double current_price,
        double r,
        double sigma,
        double q = 0.0
    );
    
    ~OptionJob() { delete option; }
    
    // Getters for private members
    inline double get_S_max() const { return S_max; }
    inline int get_J() const { return J; }
    inline int get_N() const { return N; }
    inline Option* get_option() const { return option; }

    // Comparison operators for std::set uniqueness (ticker, type, K, T)
    bool operator<(const OptionJob& other) const {
        return std::tie(ticker, option_type, K, T) < 
               std::tie(other.ticker, other.option_type, other.K, other.T);
    }
    
    bool operator==(const OptionJob& other) const {
        return ticker == other.ticker && 
               option_type == other.option_type && 
               K == other.K && 
               T == other.T;
    }

private:
    // private members since they're implementation details
    double S_max;
    int J;
    int N;
    Option* option;

    // private methods
    double calculate_S_max() const;
    int calculate_J() const;
    int calculate_N() const;
    Option* create_option();
};

class OptionJobResult {
    std::string ticker;
    std::string option_type;
    double K;
    int T;
    double current_price;
    double fair_value; // computed by the PDE solver

    OptionJobResult(
        std::string ticker,
        std::string option_type,
        double K,
        int T,
        double current_price,
        double fair_value
    ) : ticker(ticker), option_type(option_type), K(K), T(T), current_price(current_price), fair_value(fair_value) {}
};

class JobQueue {
private:
    std::queue<OptionJob> job_queue;
    std::set<OptionJob> seen_keys;
    mutable std::mutex job_queue_mutex;
public:
    void add_or_replace_job(const OptionJob& job);
    void remove_job(const OptionJob& job);
    double* run_job(const OptionJob& job);
    size_t size() const;
    OptionJob front() const;
};

class JobQueueProcessor {
private:
    JobQueue job_queue;
    std::vector<std::thread> threads;
public:
    JobQueueProcessor(size_t num_threads) : threads(num_threads) {}
    ~JobQueueProcessor() {
        stop();
        join();
    }

    OptionJobResult run_batch(const std::vector<OptionJob>& jobs, const int batch_size);
    void stop();
    void join();
};

#endif // JOB_QUEUE_H 
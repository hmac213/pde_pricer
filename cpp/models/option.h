#ifndef OPTION_H
#define OPTION_H

#include <algorithm>
#include <cmath>

class Option {
public:
    Option(double K_, double T_, double r_, double sigma_, double q_ = 0.0);

    virtual double payoff(double S) const = 0;
    virtual void option_price_boundary(double* V_time, const double* S, const double t, int size) const = 0;

    // Inline getter methods
    inline double getK() const { return K; }
    inline double getT() const { return T; }
    inline double getR() const { return r; }
    inline double getSigma() const { return sigma; }
    inline double getQ() const { return q; }

    // Inline setter methods
    inline void setK(double K_) { K = K_; }
    inline void setT(double T_) { T = T_; }
    inline void setR(double r_) { r = r_; }
    inline void setSigma(double sigma_) { sigma = sigma_; }
    inline void setQ(double q_) { q = q_; }

protected:
    double K;
    double T;
    double r;
    double sigma;
    double q;
};

class EuropeanCall : public Option {
public:
    EuropeanCall(double K_, double T_, double r_, double sigma_, double q_ = 0.0);
    
    double payoff(double S) const override;
    void option_price_boundary(double* V_time, const double* S, const double t, int size) const override;
};

class EuropeanPut : public Option {
public:
    EuropeanPut(double K_, double T_, double r_, double sigma_, double q_ = 0.0);
    
    double payoff(double S) const override;
    void option_price_boundary(double* V_time, const double* S, const double t, int size) const override;
};

#endif // OPTION_H 
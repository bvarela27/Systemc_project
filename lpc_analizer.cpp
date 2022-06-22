#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "hanning.h"
#include "lpc_analizer.h"

using namespace std;

////////////////////////////////////////////////////
// Set samples and sample_rate
void lpc_analizer::set_samples(vector<double> samples_arg, double sample_rate_arg = 44100) {
    samples = samples_arg;
    sample_rate = sample_rate_arg;

    normalize_samples();
    set_OLA();
};

////////////////////////////////////////////////////
// Normalize function
void lpc_analizer::normalize_samples() {
    // Samples abs values
    vector<double> samples_abs;
    for (int i=0; i<samples.size(); i++) {
        samples_abs.push_back(abs(samples[i]));
    }

    // Get max value
    double max_sample_value = *max_element(samples_abs.begin(), samples_abs.end());
    
    // Normalize samples
    for (int i=0; i<samples.size(); i++) {
        samples[i] = 0.9*samples[i]/max_sample_value;
    }
};

////////////////////////////////////////////////////
// Hann window 30ms
double lpc_analizer::get_window_points () {
    return floor(WINDOW_SIZE_IN_S*sample_rate);
};

double* lpc_analizer::get_hann_window (double window_points) {
    return hanning(window_points, 1);
};

////////////////////////////////////////////////////
// OLA
void lpc_analizer::set_OLA() {
    double window_points = get_window_points();
    double* hann_window = get_hann_window(window_points);

    int num_samples = samples.size();

    int step = floor(window_points*0.5);
    int count = floor((num_samples-window_points)/step) + 1;

    arma::Mat<double> OLA_int(window_points, count);
    OLA_int.zeros();

    for (int j=0; j<count; j++) {
        for (int i=0; i<window_points; i++) {
            OLA_int(i,j) = hann_window[i] * samples[(j)*step+i];
        }
    }

    OLA = OLA_int;
};

////////////////////////////////////////////////////
// LPC Analizer
tuple <arma::Mat<double>, double> lpc_analizer::compute_LPC(arma::Mat<double> x, int p) {

    int N = x.size();
    arma::Mat<double> b (N-1, 1);

    for (int i=0; i<N-1; i++) {
        b(i, 0) = x(i+1, 0);
    }

    arma::Mat<double> xz = x;
    arma::Mat<double> p_zero_matrix (p, 1);
    p_zero_matrix.zeros();

    xz.insert_rows(xz.n_rows, p_zero_matrix);

    arma::Mat<double> temp;
    arma::Mat<double> A (N-1, p);
    A.zeros();

    for (int i=0; i<p; i++) {
        temp = shift(xz, i);
        for (int j=0; j<N-1; j++) {
            A(j, i)  = temp(j, 0);
        }
    }

    // Solve
    arma::Mat<double> a = solve(A, b);

    // Calculate variance of errors
    arma::Mat<double> e = b - A*a;
    arma::Mat<double> g = var(e);

    return make_tuple(a, (double)g(0,0));
};

tuple <bool, double, vector<double>> lpc_analizer::compute_LPC_window() {
    bool valid;
    double gain;
    vector<double> coeffs;
    arma::Mat<double> coeffs_matrix;

    // There are NOT windows pending to be processed
    if (current_window >= OLA.n_cols) {
        valid = 0;
    // There are windows pending to be processed
    } else {
        valid = 1;
        tie(coeffs_matrix, gain) = compute_LPC(OLA.col(current_window), LPC_ORDER);
        //cout << coeffs_matrix.col(0) << endl;
        for (int i=0; i<LPC_ORDER; i++) {
            coeffs.push_back((double)coeffs_matrix(i, 0));
        }
        current_window++;
    }

    return make_tuple(valid, gain, coeffs);
};

// Pending
//tuple <bool, arma::Mat<double>, arma::Mat<double>> lpc_analizer::compute_LPC_all_windows()


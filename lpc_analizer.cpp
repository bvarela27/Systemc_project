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

    // Set pointer to the first window
    current_window = 0;

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

    double overlap_size = floor(window_points*(double)(OVERLAP_PERCENTAGE)/(double)(100));
    double num_windows = ((samples.size()-window_points) > 0) ? ceil((samples.size()-window_points)/overlap_size) + 1 : 1;

    arma::Mat<double> OLA_int(window_points, num_windows, arma::fill::zeros);

    for (int j=0; j<num_windows; j++) {
        for (int i=0; i<window_points; i++) {
            if ((j)*overlap_size+i < samples.size()) {
                OLA_int(i,j) = hann_window[i] * samples[(j)*overlap_size+i];
            }
        }
    }

    OLA = OLA_int;
};

////////////////////////////////////////////////////
// LPC Analizer
tuple <arma::Mat<double>, double> lpc_analizer::compute_LPC(arma::Mat<double> samples, int p) {

    int N = samples.size();
    arma::Mat<double> b (N-1, 1);

    for (int i=0; i<N-1; i++) {
        b(i, 0) = samples(i+1, 0);
    }

    arma::Mat<double> temp;
    arma::Mat<double> A (N-1, p, arma::fill::zeros);

    // Create autocorrelation matrix
    for (int i=0; i<p; i++) {
        temp = shift(samples, i);
        for (int j=0; j<N-1; j++) {
            A(j, i)  = temp(j, 0);
        }
    }

    // Solve Levinson-Gurbin recursion
    arma::Mat<double> a = solve(A, b);

    // Calculate the errors errors
    arma::Mat<double> error = b - A*a;

    // Calculate variance of errors
    arma::Mat<double> g = var(error);

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
        for (int i=0; i<coeffs_matrix.n_rows; i++) {
            coeffs.push_back((double)coeffs_matrix(i, 0));
        }
        current_window++;
    }

    return make_tuple(valid, gain, coeffs);
};

// Pending
//tuple <bool, arma::Mat<double>, arma::Mat<double>> lpc_analizer::compute_LPC_all_windows()


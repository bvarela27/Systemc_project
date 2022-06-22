#ifndef LPC_ANALIZER
#define LPC_ANALIZER

#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>

using namespace std;

////////////////////////////////
// Defines
#define LPC_ORDER           24
#define WINDOW_SIZE_IN_S    0.03

////////////////////////////////
// Module
SC_MODULE (lpc_analizer)  {
    ////////////////////////////////////////////////////
    // Internal variables
    arma::Mat<double>     OLA;
    vector<double>        samples;
    double                sample_rate;
    int                   current_window;

    ////////////////////////////////////////////////////
    // Constructor
    SC_HAS_PROCESS(lpc_analizer);
        lpc_analizer(sc_module_name lpc_analizer) : sc_module(lpc_analizer) {
        current_window = 0;
    }

    ////////////////////////////////////////////////////
    // Set samples and sample_rate
    void set_samples(vector<double> samples_arg, double sample_rate_arg);

    ////////////////////////////////////////////////////
    // Normalize function
    void normalize_samples();

    ////////////////////////////////////////////////////
    // Hann window 30ms
    double get_window_points();
    double* get_hann_window(double window_points);

    ////////////////////////////////////////////////////
    // OLA
    void set_OLA();

    ////////////////////////////////////////////////////
    // LPC Analizer
    tuple <arma::Mat<double>, double> compute_LPC(arma::Mat<double> x, int p);
    tuple <bool, double, vector<double>> compute_LPC_window();
    // Pending
    //tuple <bool, arma::Mat<double>, arma::Mat<double>> compute_LPC_all_windows()
};
#endif
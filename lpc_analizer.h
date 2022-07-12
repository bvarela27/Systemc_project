#ifndef LPC_ANALIZER
#define LPC_ANALIZER

#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;

////////////////////////////////
// Defines
#define LPC_ORDER           24
#define WINDOW_SIZE_IN_S    0.03
#define OVERLAP_PERCENTAGE  50

////////////////////////////////
// Module
SC_MODULE (lpc_analizer)  {
    ////////////////////////////////////////////////////
    // Internal variables
    arma::Mat<double>     OLA;
    vector<double>        samples;
    double                sample_rate;
    int                   current_window;
    sc_event              start_initiator;

    ////////////////////////////////////////////////////
    // TLM initiator
    tlm_utils::simple_initiator_socket<lpc_analizer> socket;

    ////////////////////////////////////////////////////
    // Constructor
    SC_HAS_PROCESS(lpc_analizer);
        lpc_analizer(sc_module_name lpc_analizer) : sc_module(lpc_analizer), socket("socket") {
        socket.register_nb_transport_bw(this, &lpc_analizer::nb_transport_bw);
        SC_THREAD(thread_process);
        current_window = 0;
    }

    ////////////////////////////////////////////////////
    // Thread process (Initiator)
    void thread_process();

    ////////////////////////////////////////////////////
    // Backward path non-blocking (Initiator)
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );

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
    // Overlap-Add
    void set_OLA();

    ////////////////////////////////////////////////////
    // LPC Analizer
    tuple <arma::Mat<double>, double> compute_LPC(arma::Mat<double> samples, int p);
    tuple <bool, double, vector<double>> compute_LPC_window();
};
#endif
#ifndef LPC_SYNTHESIS
#define LPC_SYNTHESIS

#include "systemc.h"
#include <math.h>
#include <random>
#include <armadillo>

#define WINDOW_LENGTH 240
#define WINDOW_LENGTH_2 WINDOW_LENGTH/2
#define N_POLES 24

SC_MODULE(lpc_synthesis) {
    double input_buffer[N_POLES+2];
    double LPC_output[WINDOW_LENGTH_2];

    SC_HAS_PROCESS(lpc_synthesis);
        lpc_synthesis(sc_module_name lpc_synthesis) : sc_module(lpc_synthesis){
    }

    void LPC_decoding();
    void execute(double * input);
    double* read_output();

    arma::vec filter(arma::vec b, arma::vec a, arma::vec X);
    arma::vec lcpDecode(arma::vec A, double *GFE);
};

#endif // LPC_SYNTHESIS

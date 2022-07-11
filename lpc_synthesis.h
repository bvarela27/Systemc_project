#ifndef LPC_SYNTHESIS
#define LPC_SYNTHESIS

#include "systemc.h"
#include <math.h>
#include <random>
#include <armadillo>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define WINDOW_LENGTH 240
#define WINDOW_LENGTH_2 WINDOW_LENGTH/2
#define N_POLES 24

SC_MODULE(lpc_synthesis) {
    double input_buffer[N_POLES+2];
    double LPC_output[WINDOW_LENGTH_2];

    tlm_utils::simple_target_socket<lpc_synthesis> socket;

    SC_HAS_PROCESS(lpc_synthesis);
        lpc_synthesis(sc_module_name lpc_synthesis) : sc_module(lpc_synthesis), socket("socket") {
        socket.register_b_transport(this, &lpc_synthesis::b_transport);
    }

    void LPC_decoding();
    void execute(double * input);
    double* read_output();

    arma::vec filter(arma::vec b, arma::vec a, arma::vec X);
    arma::vec lcpDecode(arma::vec A, double *GFE);

    virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay );
};

#endif // LPC_SYNTHESIS

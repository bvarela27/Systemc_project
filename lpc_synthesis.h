#ifndef LPC_SYNTHESIS
#define LPC_SYNTHESIS

#include "systemc.h"
#include <math.h>
#include <random>
#include <armadillo>
#include <queue>

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define WINDOW_LENGTH 240
#define WINDOW_LENGTH_2 WINDOW_LENGTH/2
#ifndef N_POLES
    #define N_POLES 24
#endif

using namespace std;

SC_MODULE(lpc_synthesis) {
    sc_event event_thread_process,  done;
    double input_buffer[N_POLES+2];
    double LPC_output[WINDOW_LENGTH_2];

    tlm_utils::simple_target_socket<lpc_synthesis> socket;

    // Queues
    queue<tlm::tlm_generic_payload*> trans_pending;
    tlm::tlm_phase phase_pending;

    SC_HAS_PROCESS(lpc_synthesis);
        lpc_synthesis(sc_module_name lpc_synthesis) : sc_module(lpc_synthesis), socket("socket") {
        socket.register_nb_transport_fw(this, &lpc_synthesis::nb_transport_fw);
        SC_THREAD(thread_process);
        SC_THREAD(thread_notify);
    }

    void LPC_decoding();
    void execute(double * input);
    double* read_output();

    arma::vec filter(arma::vec b, arma::vec a, arma::vec X);
    arma::vec lcpDecode(arma::vec A, double *GFE);

    void thread_process();
    void thread_notify();

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
};

#endif // LPC_SYNTHESIS

#ifndef LPC_ANALIZER
#define LPC_ANALIZER

#include "systemc.h"
#include <iostream>
#include "26bit.h"
#include "pitch.h"
#include "support_math.h"
#include "singular_value_decomposition.h"
#include "tlm.h"
#include "queue"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;

////////////////////////////////
// Defines
#define WINDOW              240
#define M                   WINDOW-1
#define LPC_ORDER           24
#ifndef N_POLES
    #define N_POLES             LPC_ORDER
#endif
#define N                   N_POLES
#define WINDOW_SIZE_IN_S    0.03
#define OVERLAP_PERCENTAGE  50

////////////////////////////////
// Module
SC_MODULE (lpc_analizer)  {

    // Internal variables
    double                  stacked[WINDOW];
    double                  samples[WINDOW];
    double                  samples_normalized[WINDOW];
    int                     current_window, sample_number;

    sc_event                start_initiator, event_thread_process;

    queue<tlm::tlm_generic_payload*> trans_pending;

    // TLM initiator
    tlm_utils::simple_target_socket<lpc_analizer> target_socket;
    tlm_utils::simple_initiator_socket<lpc_analizer> initiator_socket;

    // Constructor
    SC_HAS_PROCESS(lpc_analizer);
        lpc_analizer(sc_module_name lpc_analizer) : sc_module(lpc_analizer), initiator_socket("initiator_socket"), target_socket("target_socket") {
        target_socket.register_nb_transport_fw(this, &lpc_analizer::nb_transport_fw);
        initiator_socket.register_nb_transport_bw(this, &lpc_analizer::nb_transport_bw);
        SC_THREAD(thread_process);
        SC_THREAD(thread_process_resp);
        current_window = 0;
        sample_number = 0;
    }

    // Thread process (Initiator)
    void thread_process();
    void thread_process_resp();

    // Backward path non-blocking (Initiator)
    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );

    // Set samples and sample_rate
    void set_sample(double sample);

    // Normalize function
    void normalize_samples();

    // Overlap-Add
    void set_OLA();

    // LPC Analizer
    void compute_LPC(double* poles_gain);
    void compute_LPC_window(int32_t* poles_gain_pitch);
};
#endif

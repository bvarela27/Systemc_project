#ifndef HAMMING_ENCODER
#define HAMMING_ENCODER

#include <iostream>
#include <systemc.h>
#include <stdlib.h>
#include <queue>
#include "register_map.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define N_HAMMING 31
#define K_HAMMING 26
#define M_HAMMING N_HAMMING-K_HAMMING

using namespace std;

SC_MODULE(HammingEnc) {
    sc_event enc_t, event_thread_process, done;
    sc_bv<32> reg = 0;
    sc_bv<32> temp = 0;

    // Queues
    queue<tlm::tlm_generic_payload*> trans_pending;
    tlm::tlm_phase phase_pending;
    tlm_utils::simple_target_socket<HammingEnc> target_socket;
    tlm_utils::simple_initiator_socket<HammingEnc> initiator_socket;

    bool waiting_response = false;

    SC_CTOR(HammingEnc): target_socket("target_socket"), initiator_socket("initiator_socket") {
        target_socket.register_nb_transport_fw(this, &HammingEnc::nb_transport_fw);
        initiator_socket.register_nb_transport_bw(this, &HammingEnc::nb_transport_bw);
        SC_THREAD(encode);
        SC_THREAD(thread_process);
        SC_THREAD(thread_notify);
    }

    void thread_process();
    void thread_notify();

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );

    void encode();

    void write(uint32_t in);

    uint32_t read();
};

#endif

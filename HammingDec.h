#ifndef HAMMING_DECODER
#define HAMMING_DECODER

#include <iostream>
#include <systemc.h>
#include <stdlib.h>
#include <list>
#include "register_map.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define N_HAMMING 31
#define K_HAMMING 26
#define M_HAMMING N_HAMMING-K_HAMMING

using namespace std;

SC_MODULE(HammingDec) {
    sc_event ecc_t, dec_t, event_thread_process, done;
    sc_signal< sc_uint<32> > in;
    sc_signal< sc_uint<32> > out;
    sc_bv<32> reg, decoded_val;
    sc_bv<32> syn;

    // Queues
    list<tlm::tlm_generic_payload*> queue_trans_pending;

    tlm_utils::simple_target_socket<HammingDec> target_socket;
    tlm_utils::simple_initiator_socket<HammingDec> initiator_socket;

    SC_CTOR(HammingDec): target_socket("target_socket"), initiator_socket("initiator_socket") {
        target_socket.register_nb_transport_fw(this, &HammingDec::nb_transport_fw);
        initiator_socket.register_nb_transport_bw(this, &HammingDec::nb_transport_bw);
        SC_THREAD(thread_process);
        SC_THREAD(thread_notify);
        SC_THREAD(decode);
        SC_THREAD(ECC);
    }

    void thread_process();
    void thread_notify();

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );

    void decode();

    void ECC();

    void write();
};

#endif
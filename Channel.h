#ifndef CHANNEL_H
#define CHANNEL_H

#include "systemc.h"
#include "systemc-ams.h"

#include <iostream>
#include <stdlib.h>
#include <queue>
#include "bask_mod.h"
#include "bask_demod.h"
#include "protocol_gen.h"
#include "protocol_det.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;

SC_MODULE(Channel) {
    // AMS components
    protocol_gen prot_gen0;
    bask_mod mod0;
    bask_demod demod0;
    protocol_det prot_det0;

    // AMS signals
    sca_tdf::sca_signal<bool> prot_gen_out, demod_out;
    sca_tdf::sca_signal<double> mod_out;

    // Events
    sc_event event_thread_process, done;

    // Queues
    queue<tlm::tlm_generic_payload*> trans_pending;

    // Sockets
    tlm_utils::simple_target_socket<Channel> target_socket;
    tlm_utils::simple_initiator_socket<Channel> initiator_socket;

    SC_CTOR(Channel): target_socket("target_socket"), initiator_socket("initiator_socket"),
        prot_gen0("protocol_gen"), mod0("bask_mod"),
        demod0("bask_demod"), prot_det0("protocol_det") {
        // AMS Connections
        prot_gen0.out(prot_gen_out);

        mod0.in(prot_gen_out);
        mod0.out(mod_out);

        demod0.in(mod_out);
        demod0.out(demod_out);

        prot_det0.in(demod_out);

        // Sockets
        target_socket.register_nb_transport_fw(this, &Channel::nb_transport_fw);
        initiator_socket.register_nb_transport_bw(this, &Channel::nb_transport_bw);

        // Threads
        SC_THREAD(thread_process);
        SC_THREAD(io_request);
        SC_THREAD(thread_notify);
    }

    void io_request();
    void thread_process();
    void thread_notify();

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
};

#endif
#ifndef GENERIC_INITIATOR_TARGET
#define GENERIC_INITIATOR_TARGET

#include <iostream>
#include "systemc.h"
#include <stdlib.h>
#include <queue>
#include "register_map.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

using namespace std;

SC_MODULE(generic_initiator_target) {
    sc_event event_thread_process;

    // Queues
    queue<tlm::tlm_generic_payload*> trans_pending;
    tlm_utils::simple_target_socket<generic_initiator_target> target_socket;
    tlm_utils::simple_initiator_socket<generic_initiator_target> initiator_socket;

    SC_CTOR(generic_initiator_target): target_socket("target_socket"), initiator_socket("initiator_socket") {
        target_socket.register_nb_transport_fw(this, &generic_initiator_target::nb_transport_fw);
        initiator_socket.register_nb_transport_bw(this, &generic_initiator_target::nb_transport_bw);
        SC_THREAD(thread_process);
    }

    void thread_process();

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
};

#endif

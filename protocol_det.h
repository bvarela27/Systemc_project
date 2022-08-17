#ifndef PROTOCOL_DET_H
#define PROTOCOL_DET_H

#include <queue>
#include "ams_shared.h"
#include "systemc-ams.h"

using namespace std;

SCA_TDF_MODULE(protocol_det) {
    // Events to trigger the TLM logic
    sc_event io_request;

    // Data encoded received (This has to be a public variable)
    sc_bv<NUM_BITS_DATA_ENCODED> data_encoded_received;

    sca_tdf::sca_in<bool> in;

    SCA_CTOR(protocol_det): in("in") {

    }

    void initialize();

    void processing();

    private:
        State state;
        int bit_idx;

};

#endif

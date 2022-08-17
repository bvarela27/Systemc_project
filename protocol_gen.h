#ifndef PROTOCOL_GEN_H
#define PROTOCOL_GEN_H

#include <queue>
#include "ams_shared.h"
#include "systemc-ams.h"

using namespace std;

SCA_TDF_MODULE(protocol_gen) {
    sca_tdf::sca_out<bool> out; // output port

    SCA_CTOR(protocol_gen): out("out") {

    }

    void initialize();

    void processing();

    void store_data(uint32_t data_encoded);

    private:
        State state;
        int bit_idx;
        sc_bv<NUM_BITS_DATA_ENCODED> data_encoded;
        queue<sc_bv<NUM_BITS_DATA_ENCODED>> queue_data_encoded;

};

#endif

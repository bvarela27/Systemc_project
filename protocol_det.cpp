#include <stdlib.h>
#include "protocol_det.h"

void protocol_det::initialize() {
    state = IDLE;
    bit_idx = 0;
    data_encoded_received = 0;
}

void protocol_det::processing() {

    if (state == IDLE) {
        if (in.read() == START_BIT) {
            state = START;

            bit_idx = 0;
        }
    } else if (state == START) {
        state = IN_PKT;

        data_encoded_received[bit_idx] = in.read();
        bit_idx++;
    } else {
        if (bit_idx == NUM_BITS_DATA_ENCODED) {
            state = IDLE;

            bit_idx = 0;

            // FIXME remove this later
            //cout << "DATA RECEIVED: " << ((sc_uint<NUM_BITS_DATA_ENCODED>)data_encoded_received) << " at time " << sc_time_stamp() << endl;

            // Trigger event
            io_request.notify();
        } else {
            data_encoded_received[bit_idx] = in.read();
            bit_idx++;
        }
    }
}

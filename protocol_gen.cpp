#include "protocol_gen.h"

void protocol_gen::initialize() {
    state = IDLE;
    bit_idx = 0;
    data_encoded = 0;

}

void protocol_gen::store_data(uint32_t data_encoded_) {
    sc_bv<NUM_BITS_DATA_ENCODED> tmp = (sc_bv<NUM_BITS_DATA_ENCODED>) data_encoded_;
    queue_data_encoded.push(tmp);
}

void protocol_gen::processing() {
    if (state == IDLE) {
        if (queue_data_encoded.empty()) {
            out.write(IDLE_BIT);
        } else {
            state = START;

            out.write(START_BIT);
        }
    } else if (state == START) {
        state = IN_PKT;

        data_encoded = queue_data_encoded.front();
        queue_data_encoded.pop();

        out.write((bool) data_encoded[bit_idx]);
        bit_idx++;
    } else {
        if (bit_idx == NUM_BITS_DATA_ENCODED) {
            state = IDLE;

            out.write(IDLE_BIT);

            bit_idx = 0;
        } else {
            out.write((bool) data_encoded[bit_idx]);
            bit_idx++;
        }
    }
}

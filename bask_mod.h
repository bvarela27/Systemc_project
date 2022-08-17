#ifndef BASK_MOD_H
#define BASK_MOD_H

#include "systemc-ams.h"

#include "mixer.h"
#include "carrier.h"

#define DEFAULT_CARRIER_FREQUENCY   0.1e7

SC_MODULE(bask_mod) {
    sca_tdf::sca_in<bool>    in;
    sca_tdf::sca_out<double> out;

    carrier carrier0;
    mixer   mix;

    SC_CTOR(bask_mod): in("in"),out("out"),
        carrier0("carrier", DEFAULT_CARRIER_FREQUENCY, sca_core::sca_time( 100.0, sc_core::SC_NS ) ),
        mix("mix") {

        // Connections
        carrier0.out(carrier_signal);
        mix.in_wav(carrier_signal);
        mix.in_bin(in);
        mix.out(out);
    }

    private:
        sca_tdf::sca_signal<double> carrier_signal;
};

#endif
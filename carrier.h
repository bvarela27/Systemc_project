#ifndef CARRIER_H
#define CARRIER_H

#include "systemc-ams.h"

SCA_TDF_MODULE(carrier) {
    sca_tdf::sca_out<double> out;

    carrier( sc_core::sc_module_name nm, double freq_,
        sca_core::sca_time Tm_ = sca_core::sca_time(0.125, sc_core::SC_MS))
        : out("out"), ampl(1.0), freq(freq_), Tm(Tm_) {

    }

    void set_attributes();

    void processing();

    void set_frequency(double freq_);

    private:
        double ampl;            // amplitude
        double freq;            // frequency
        sca_core::sca_time Tm;  // module time step
};

#endif

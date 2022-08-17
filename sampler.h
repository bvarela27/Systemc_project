#ifndef SAMPLER_H
#define SAMPLER_H

#include "systemc-ams.h"

SCA_TDF_MODULE(sampler) {
    sca_tdf::sca_in<double> in;   // input port
    sca_tdf::sca_out<bool>  out;  // output port

    sampler(sc_core::sc_module_name nm, double threshold_)
        : in("in"),out("out"), rate(20), threshold(threshold_) {

    }

    void set_attributes();

    void processing();

    void set_threshold(double threshold_);

    private:
        unsigned long rate;
        double threshold;
        int sample_pos;
};

#endif

#ifndef DEFINE_FILTER_H
#define DEFINE_FILTER_H

#include "systemc-ams.h"

SCA_TDF_MODULE(filter) {
    sca_tdf::sca_in<double>     in;
    sca_tdf::sca_out<double>    out;

    filter(sc_core::sc_module_name nm, double fc_,
        double gain_ = 1.0)
        : in("in"), out("out"), fc(fc_), gain(gain_) {
    }

    void set_gain(double gain_);

    void set_cutoff_frequency(double fc_);

    void initialize();

    void processing();

    private:
        sca_tdf::sca_ltf_nd ltf_nd;             // Laplace transfer function
        sca_util::sca_vector<double> num, den;  // numerator and denominator coefficients
        double fc;                              // 3dB cutoff frequency in Hz
        double gain;                            // DC gain
};

#endif
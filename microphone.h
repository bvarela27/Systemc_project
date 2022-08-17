#ifndef DEFINE_AUDIO_H
#define DEFINE_AUDIO_H

#define FACTOR 10000

#include <vector>
#include "systemc-ams.h"

using namespace std;

SCA_TDF_MODULE (microphone) {
    sca_tdf::sca_out<double> out;

    microphone( sc_core::sc_module_name nm,
        sca_core::sca_time Tm_ = sca_core::sca_time(0.125, sc_core::SC_MS))
        : out("out"), index(0), Tm(Tm_) {
    }

    void initialize();

    void set_attributes();

    void processing();

    private:
        int index;
        vector<double> samples;
        sca_core::sca_time Tm; // module time step
};

#endif
#ifndef BASK_DEMOD_H
#define BASK_DEMOD_H

#include "systemc-ams.h"

#include "rectifier.h"
#include "filter.h"
#include "sampler.h"

#define DEFAULT_FILTER_GAIN                 1.0
#define DEFAULT_FILTER_CUTOFF_FREQUENCY     0.33e6
#define DEFAULT_SAMPLER_THRESHOLD           0.2

SC_MODULE(bask_demod) {
    sca_tdf::sca_in<double>     in;
    sca_tdf::sca_out<bool> 	    out;

    rectifier     rc;
    filter        lp;
    sampler       sp;

    SC_CTOR(bask_demod): in("in"), out("out"), rc("rc"),
        lp("lp", DEFAULT_FILTER_CUTOFF_FREQUENCY, DEFAULT_FILTER_GAIN),
        sp("sp", DEFAULT_SAMPLER_THRESHOLD), rc_out("rc_out"), lp_out("lp_out") {
        rc.in(in);
        rc.out(rc_out);

        lp.in(rc_out);
        lp.out(lp_out);

        sp.in(lp_out);
        sp.out(out);
    }

    private:
        sca_tdf::sca_signal<double> rc_out, lp_out;
};

#endif
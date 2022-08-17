#include "filter_decoup.h"

void filter_decoup::set_gain(double gain_) {
    gain = gain_;
}

void filter_decoup::set_cutoff_frequency(double fc_) {
    fc = fc_;
    den(1) = 1.0 / ( 2.0* M_PI * fc );
}

void filter_decoup::initialize() {
    num(0) = 1.0;
    den(0) = 1.0;
    den(1) = 1.0 / ( 2.0* M_PI * fc );
}
 
void filter_decoup::processing() {
    double tmp = ltf_nd( num, den, in.read(), gain );
    out.write(tmp);
}

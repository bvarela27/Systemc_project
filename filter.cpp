#include "filter.h"

void filter::set_gain(double gain_) {
    gain = gain_;
}

void filter::set_cutoff_frequency(double fc_) {
    fc = fc_;
    den(1) = 1.0 / ( 2.0* M_PI * fc );
}

void filter::initialize() {
    num(0) = 1.0;
    den(0) = 1.0;
    den(1) = 1.0 / ( 2.0* M_PI * fc );
}
 
void filter::processing() {
    double tmp = ltf_nd( num, den, in.read(), gain );
    out.write(tmp);
}

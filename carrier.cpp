#include "carrier.h"

void carrier::set_frequency(double freq_) {
    freq = freq_;
}

void carrier::set_attributes() {
    set_timestep(Tm);
}

void carrier::processing() {
    double t = get_time().to_seconds();
    out.write( ampl * std::sin( 2.0 * M_PI * freq * t ) );
}


#include "sampler.h"

void sampler::set_threshold(double threshold_) {
    threshold = threshold_;
}

void sampler::set_attributes() {
    in.set_rate(rate);
    sample_pos = (int)std::ceil( 2.0 * (double)rate/3.0 );
}
	
void sampler::processing() {
    double t = get_time().to_seconds();

    if( in.read(sample_pos) > threshold ) {
        out.write(true);
    } else {
        out.write(false);
    }
}

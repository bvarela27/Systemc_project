#include "rectifier.h"

void rectifier::processing() {
    out.write( std::abs(in.read()) );
}


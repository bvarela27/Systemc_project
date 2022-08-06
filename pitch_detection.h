#ifndef PITCH_DETECTION
#define PITCH_DETECTION

#include <armadillo>
using namespace arma;

void findpeaks (vec& X, ivec& locs);
double get_Pitch (vec& X);

#endif

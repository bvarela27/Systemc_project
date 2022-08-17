/* Fast Fourier Transform
 * Cooley-Tukey algorithm with 2-radix DFT
 */
#ifndef FFT
#define FFT
#include <math.h>

void fft(double data_re[], double data_im[], const unsigned int N);

void rearrange(double data_re[], double data_im[], const unsigned int N);

void compute(double data_re[], double data_im[], const unsigned int N);
#endif

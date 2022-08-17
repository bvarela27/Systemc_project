#ifndef PITCH_FUNC
#define PITCH_FUNC
#include <math.h>
#include <string.h>
#include "support_math.h"
#include "fft.h"

#define IN_WINDOW_LENGHT 240
#define N_PEAKS 5
#define N_FFT 1024
#define FREQ 8000

double find_pitch(double * window, double * f, int window_lenght);
void frequency(double * f, double freq);
void find_peaks(double * window, int lenght, int * locs);
void norm_abs(double * re_array, double * im_array, double * norm, double lenght);
int min_value_idx(double * array, int lenght);
int compare( const void* a, const void* b);
#endif

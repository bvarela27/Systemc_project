#ifndef SUPPORT_MATH
#define SUPPORT_MATH

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 	3.14159265358979323846

void zeros(double * array, int size);
void scale(double * array, double value, int size);
#ifdef DEBUG
void print_array(double * array, int size);
void print_int_array(int * array, int size);
#endif
double rand_gen();
double normalRandom();
void randn_array(double * array, int size);
void hanning(double * window, int lenght);
void Subtract_Matrices(double *C, double *A, double *B, int nrows, int ncols);
void Multiply_Matrices(double *C, double *A, int nrows, int ncols, double *B, int mcols);
void Transpose_Matrix(double *At, double *A, int nrows, int ncols);
double variance(double * array, int lenght);
#endif

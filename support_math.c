#include "support_math.h"

void zeros(double * array, int size)
{
    for(int i = 0; i<size; i++)
    {
        array[i] = 0;
    }
}

void scale(double * array, double scale_value, int size)
{
    for(int i = 0; i<size; i++)
    {
        array[i] = array[i]/scale_value;
    }
}

void print_array(double * array, int size)
{
    for(int i = 0; i<size; i++)
    {
        printf("%d ---- %.10f\n",i,array[i]);
    }
}

void print_int_array(int * array, int size)
{
    for(int i = 0; i<size; i++)
    {
        printf("%d\n",array[i]);
    }
}

double rand_gen()
{
   return ( (double)(rand()) + 1. )/( (double)(RAND_MAX) + 1. );
}

double normalRandom()
{
   double v1=rand_gen();
   double v2=rand_gen();
   return cos(2*3.14*v2)*sqrt(-2.*log(v1));
}

void randn_array(double * array, int size)
{
    for(int i=0;i<size;i++)
    {
        array[i] = normalRandom();
    }
}

void hanning(double * window, int lenght)
{
    for(int i = 0; i < lenght; i++)
    {
        window[i] = 0.5*(1-cos(2*PI*i/(lenght)));
    }
}

void Subtract_Matrices(double *C, double *A, double *B, int nrows,
                                                                    int ncols)
{
   register int i;
   register int n = nrows * ncols;

   for (i = 0; i < n; i++) C[i] = A[i] - B[i];
}

void Multiply_Matrices(double *C, double *A, int nrows, int ncols,
                                                          double *B, int mcols)
{
   double *pB;
   double *p_B;
   int i,j,k;

   for (i = 0; i < nrows; A += ncols, i++)
      for (p_B = B, j = 0; j < mcols; C++, p_B++, j++) {
         pB = p_B;
         *C = 0.0;
         for (k = 0; k < ncols; pB += mcols, k++)
            *C += *(A+k) * *pB;
      }
}

void Transpose_Matrix(double *At, double *A, int nrows, int ncols)
{
   double *pA;
   double *pAt;
   int i,j;

   for (i = 0; i < nrows; At += 1, A += ncols, i++) {
      pAt = At;
      pA = A;
      for (j = 0; j < ncols; pAt += nrows, j++) *pAt = pA[j];
   }
}

double variance(double * array, int lenght)
{
    double var= 0;

    double mean = 0;

    for(int i = 0; i < lenght; i++)
    {
        mean += array[i];
    }
    mean = mean/(double)lenght;

    for(int i = 0; i < lenght; i++)
    {
        var += pow((array[i] - mean),2);
    }
    var = var/(double)lenght;

    return var;
}



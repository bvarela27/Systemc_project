#ifndef HANNING
#define HANNING

/*  function w = hanning(varargin)
%   HANNING   Hanning window.
%   HANNING(N) returns the N-point symmetric Hanning window in a column
%   vector.  Note that the first and last zero-weighted window samples
%   are not included.
%
%   HANNING(N,'symmetric') returns the same result as HANNING(N).
%
%   HANNING(N,'periodic') returns the N-point periodic Hanning window,
%   and includes the first zero-weighted window sample.
%
%   NOTE: Use the HANN function to get a Hanning window which has the
%          first and last zero-weighted samples.ep
    itype = 1 --> periodic
    itype = 0 --> symmetric
    default itype=0 (symmetric)

    Copyright 1988-2004 The MathWorks, Inc.
%   $Revision: 1.11.4.3 $  $Date: 2007/12/14 15:05:04 $
*/

#include <math.h>

double* hanning(int N, short itype) {
    int half, i, idx, n;
    double *w;

    w = (double*) calloc(N, sizeof(double));
    memset(w, 0, N*sizeof(double));

    if(itype==1)    //periodic function
        n = N-1;
    else
        n = N;

    if(n%2==0)
    {
        half = n/2;
        for(i=0; i<half; i++) //CALC_HANNING   Calculates Hanning window samples.
            w[i] = 0.5 * (1 - cos(2*M_PI*(i+1) / (n+1)));

        idx = half-1;
        for(i=half; i<n; i++) {
            w[i] = w[idx];
            idx--;
        }
    }
    else
    {
        half = (n+1)/2;
        for(i=0; i<half; i++) //CALC_HANNING   Calculates Hanning window samples.
            w[i] = 0.5 * (1 - cos(2*M_PI*(i+1) / (n+1)));

        idx = half-2;
        for(i=half; i<n; i++) {
            w[i] = w[idx];
            idx--;
        }
    }

    if(itype==1)    //periodic function
    {
        for(i=N-1; i>=1; i--)
            w[i] = w[i-1];
        w[0] = 0.0;
    }
    return(w);
}
#endif
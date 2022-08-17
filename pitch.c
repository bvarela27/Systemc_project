#include "pitch.h"

double find_pitch(double * window, double * f, int window_lenght)
{
    double fft_norm[N_FFT];
    double re_window[N_FFT] = {0};
    double im_window[N_FFT] = {0};
    memcpy(re_window,window,window_lenght*sizeof(double));
    fft( re_window, im_window, N_FFT);

    norm_abs( re_window, im_window, fft_norm, N_FFT);

    //print_array(fft_norm,N_FFT/2);

    int locs[N_PEAKS] = {-1,-1,-1,-1,-1};

    find_peaks(fft_norm, N_FFT/2, locs);

    //print_int_array(locs,N_PEAKS);

    int peaks_found = 0;
    double pf[N_PEAKS];
    for(int i = 0; i < N_PEAKS; i++)
    {
        if(locs[i] == -1) break;
        pf[i] = f[locs[i]];
        peaks_found ++;
    }
    //print_array(pf,peaks_found);
    //printf("----------%f\n",f[44]);

    qsort(pf, peaks_found,sizeof(double), compare);

    double power = 0;

    for(int i = 0; i < window_lenght; i++)
    {
        power+= pow(window[i],2);
    }

    power = 10 * log10(power/window_lenght);

    double f_0 = pf[0];
    double tff[peaks_found];

    for(int i = 0; i < peaks_found; i++)
    {
        tff[i] = pf[i]/f_0 - (i+1);
    }

    double ddf = fabs(tff[0]) > fabs(tff[1])? fabs(tff[0]) : fabs(tff[1]);

    //print_array(tff,peaks_found);

    //printf("%f",power);
    f_0 = (ddf >0.5 || f_0 >0.125 || power < -30)? 0: f_0;

    //print_array(tff,peaks_found);


    return f_0;
}

int compare( const void* a, const void* b)
{
     double d_a = * ( (double*) a );
     double d_b = * ( (double*) b );

     if ( d_a == d_b ) return 0;
     else if ( d_a < d_b ) return -1;
     else return 1;
}

void frequency(double * f, double freq)
{
    for (int i = 0; i < N_FFT; i++)
    {
        double step = (1.0/N_FFT)*freq;

        for(int i = 0; i < N_FFT; i++)
        {
            f[i] = step*(double)i;
        }
    }
}


void find_peaks(double * window, int lenght, int * locs)
{
    int curr_direction;
    int prev_direction;
    double prev_val = window[0];
    double curr_val = window[1];
    prev_direction = prev_val < curr_val? 1:-1;
    prev_val = window[1];
    double peaks[N_PEAKS] = {0};
    int peak_count = 0;
    for(int i = 2; i <lenght; i++)
    {
        curr_val = window[i];
        curr_direction = prev_val < curr_val? 1:-1;

        if((curr_direction != prev_direction) && (prev_direction == 1))
        {
            peak_count++;
            int idx = min_value_idx(peaks,N_PEAKS);
            if((peak_count < 5)  || (peak_count>=5 && prev_val > peaks[idx]) )
            {
                peaks[idx] = prev_val;
                locs[idx] = i-1;
            }
        }

        prev_direction = curr_direction;
        prev_val = curr_val;
    }
}

void norm_abs(double * re_array,double * im_array, double * norm, double lenght)
{
    norm[0] = sqrt(pow(re_array[0],2)+pow(im_array[0],2));

    double max_val = norm[0];

    for(int i = 1; i < lenght; i++)
    {
        norm[i] = sqrt(pow(re_array[i],2)+pow(im_array[i],2));

        if (norm[i] > max_val)
        {
            max_val = norm[i];
        }
    }

    for(int i = 0; i < lenght; i++)
    {
        norm[i] = norm[i]/max_val;
    }
}

int min_value_idx(double * array, int lenght)
{
    int idx = 0;
    double min_value = array[0];

    for(int i = 1; i < lenght; i++)
    {
        if(array[i]<min_value)
        {
            min_value = array[i];
            idx = i;
        }
    }

    return idx;
}

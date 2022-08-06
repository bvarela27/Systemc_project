#include <math.h>
#include <armadillo>
#include <map>
#include <iterator>
#include "pitch_detection.h"
using namespace arma;
using namespace std;

/**
 * @brief Find the first 5 peak frequencies from X
 * 
 * @param X Input window of frequencies
 * @param locs Vector to store the indexes of first 5 peak frequencies
 */
void findpeaks (vec& X, ivec& locs)
{
    locs = {-1, -1, -1, -1, -1};
    multimap<float, int, greater<float> > locs_map;
    multimap<float, int>::iterator it;
    int counter = 0;
    int j;
    for (j=1; j<(X.size()-1); j++) { 
        if ( X(j-1)<X(j) && X(j)>X(j+1) ) {
            locs_map.insert(make_pair(X(j), j));
        }
    }

    if (locs_map.size() < 5) {
        for (it = locs_map.begin(); it != locs_map.end(); counter++, it++) {
            locs(counter) = (*it).second;
        }
    } else {
        for (it = locs_map.begin(); counter < 5; counter++, it++) {
            locs(counter) = (*it).second;
        }
    }
  
    return;
}


/**
 * @brief Get the pitch of an input vector of samples
 * 
 * @param X Input window of samples
 * @return float Formant frequency
 */
double get_Pitch (vec& X)
{
    vec Y;
    vec f;
    ivec locs(5);
    vec pf(5);
    vec tff;
    double p;
    double ddf;
    double ff = 0;

    Y = abs( fft( X, 1024 ) );

    f = linspace<vec>(0, 1023, 1024);
    f /= 1024;

    // // We only need the lower half  
    Y = Y( span( 0, (1024/2)-1 ) );
    Y /= max(Y); // normalize

    // find first 5 peak frequencies indexes
    findpeaks(Y, locs);

    pf = {f(locs(0)), f(locs(1)), f(locs(2)), f(locs(3)), f(locs(4))};
    pf = sort(pf);
    
    p = 10*log10(mean(square(X)));

    tff = pf / pf(0);

    tff = tff - linspace<vec>(1, 5, 5);

    tff = tff(span(0, 1));

    ff = pf(0);
    ddf = max(abs(tff));

    if (ddf>0.5 || ff>0.125 || p<-30) {
        ff = 0;
    }

    return ff;
}

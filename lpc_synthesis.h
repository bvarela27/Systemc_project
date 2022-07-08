#ifndef LPC_H_INCLUDED
#define LPC_H_INCLUDED

#include <math.h>
#include <random>
#include <armadillo>

#define WINDOW_LENGTH 240
#define WINDOW_LENGTH_2 WINDOW_LENGTH/2
#define N_POLES 24

//const arma::vec b_butterworth = {0.7779,-7.7793,35.0070, -93.3519,163.3658,-196.0390,163.3658,-93.3519,35.0070,-7.7793,0.7779};
//const arma::vec a_butterworth = {1.0000,-9.4979,40.6070,-102.9696,171.2005,-195.3534,154.8446,-84.1849,30.0441,-6.3556,0.6052};

arma::vec filter(arma::vec b, arma::vec a, arma::vec X);
arma::vec lcpDecode(arma::vec A, double *GFE);

#endif // LPC_H_INCLUDED

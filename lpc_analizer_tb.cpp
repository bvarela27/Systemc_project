#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "AudioFile.h"
#include "lpc_analizer.h"

using namespace std;

int sc_main(int argc, char* argv[])  {

    lpc_analizer lpc_analizer("LPC_ANALIZER");

    sc_start();

    cout << "Testbench LPC analizer" << endl;
    cout << "@" << sc_time_stamp() << " Simulation Start" << endl;

    ////////////////////////////////////////////////////
    // Read WAV file
    AudioFile<double> audioFile;
    audioFile.load("speech.wav");

    int channel = 0;
    int sample_rate = (int) audioFile.getSampleRate();
    vector<double> samples = audioFile.samples[channel];

    ////////////////////////////////////////////////////
    // Set Samples and rate
    lpc_analizer.set_samples(samples, sample_rate);

    ////////////////////////////////////////////////////
    // Get coeffs and samples from first window
    bool valid;
    double gain;
    vector<double> coeffs;

    tie(valid, gain, coeffs) = lpc_analizer.compute_LPC_window();

    cout << "Valid: " << valid << " Gain: " << gain << endl;
    for (int i=0; i<LPC_ORDER; i++) {
        cout << "Coeff[" << i << "]: " << coeffs[i] << endl;
    }
    cout << "@" << sc_time_stamp() << " Simulation End" << endl;

    return 0;
}

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
    //int sample_rate = (int) audioFile.getSampleRate(); //FIXME resample the speech at 8KHz.
    int sample_rate = 8000;
    vector<double> samples = audioFile.samples[channel];

    ////////////////////////////////////////////////////
    // Check the amount of windows and the amount of coeffs returned per window
    bool valid;
    double gain;
    vector<double> coeffs;

    double window_points = sample_rate*WINDOW_SIZE_IN_S;
    double overlap_size = floor(window_points*OVERLAP_PERCENTAGE/100);
    int rnd_num_samples;
    int num_windows_expected;
    int count_windows;
    int count_test_pass = 0;
    int count_test_fail = 0;

    // Number of times the test will run
    int num_tests = 100;

    for (int i=0; i<num_tests; i++) {
        // Set samples and rate
        vector<double> samples_int;
        rnd_num_samples = rand() % samples.size() + 1;
        for (int j=0; j<rnd_num_samples; j++) {
            samples_int.push_back(samples[j]);
        }
        lpc_analizer.set_samples(samples_int, sample_rate);

        // Initiate counters
        count_windows = 0;

        // Calculate the expected number of windows
        if ((samples_int.size()-window_points) < 0) {
            num_windows_expected = 1;
        } else {
            num_windows_expected = 1 + ceil((samples_int.size()-window_points)/overlap_size);
        }

        while (true) {
            tie(valid, gain, coeffs) = lpc_analizer.compute_LPC_window();
            if (valid) {
                // Check the amount coeffs that were returned
                if (coeffs.size() != LPC_ORDER) {
                    cout << "\033[1;31mTest " << i << " Failed: The amount coeffs returned do not match the number of coeffs expected.\033[0m\n";
                    count_test_fail++;
                    break;
                }
                count_windows++;
            } else {
                // Check the number of windows returned
                if (count_windows != num_windows_expected) {
                    cout << "\033[1;31mTest " << i << " Failed: The number of windows returned do not match the number of windows expected.\033[0m\n";
                    count_test_fail++;
                } else {
                    cout << "\033[1;32mTest " << i << " Passed.\033[0m\n";
                    count_test_pass++;
                }
                break;
            }
        }
    }

    // Summary
    cout << "\033[1;34m\nNUMBER OF TESTS: " << num_tests << ".\033[0m\n";
    cout << "\033[1;34mNUMBER OF TESTS THAT PASSED: " << count_test_pass << ".\033[0m\n";
    cout << "\033[1;34mNUMBER OF TESTS THAT FAILED: " << count_test_fail << ".\033[0m\n";

    cout << "@" << sc_time_stamp() << " Simulation End" << endl;

    return 0;
}

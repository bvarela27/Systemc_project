#include "microphone.h"
#include "AudioFile.h"
#include <math.h>

void microphone::initialize() {
    // Read WAV file
    AudioFile<double> audioFile;
    audioFile.load("speech.wav");

    int channel = 0;
    samples = audioFile.samples[channel];
}

void microphone::set_attributes() {
    set_timestep(Tm);
}

void microphone::processing() {
    if (index >= samples.size()) {
        out.write(0.0);
    } else {
        out.write(samples[index]*(pow(2, 32-1)-1));
        //cout << "Time: " << get_time().to_seconds() << " Samples[" << index << "]: " << samples[index]*pow(2, 32-1) << endl;
        index++;
    }
}

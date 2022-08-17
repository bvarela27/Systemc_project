#include "systemc.h"
#include "systemc-ams.h"

#include "AudioCapture.h"
#include "generic_initiator_target.h"
#include <math.h>

int sc_main (int argc, char* argv[]) {
    ////////////////////////////////////////////////////
    // Audio Capture
    AudioCapture AudioCapture0("AudioCapture");

    ////////////////////////////////////////////////////
    // generic_initiator_target
    generic_initiator_target generic_initiator_target0("generic_initiator_target");

    ////////////////////////////////////////////////////
    // Router
    AudioCapture0.initiator_socket.bind( generic_initiator_target0.target_socket );
    generic_initiator_target0.initiator_socket.bind( AudioCapture0.target_socket );

    ////////////////////////////////////////////////////
    // VCD File
    sca_util::sca_trace_file *vcdfile= sca_util::sca_create_vcd_trace_file("audio_capture.vcd");
    sca_trace(vcdfile, AudioCapture0.microphone_out, "microphone_out");
    sca_trace(vcdfile, AudioCapture0.filter_out, "filter_out");
    sca_trace(vcdfile, AudioCapture0.adc_out, "adc_out");

    sc_start(1000, sc_core::SC_MS);

    sca_util::sca_close_vcd_trace_file(vcdfile);

	return 0;
}

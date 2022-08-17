#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#include <iostream>
#include "systemc.h"
#include "systemc-ams.h"
#include <stdlib.h>
#include <queue>
#include "register_map.h"
#include "adc_converter.h"
#include "filter_decoup.h"
#include "microphone.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define DEFAULT_MICROPHONE_SAMPLE_FREQUENCY 44100.0
#define DEFAULT_FILTER_DEC_CUTOFF_FREQUENCY     18000.0
#define DEFAULT_FILTER_DEC_GAIN                 1.0
#define DEFAULT_ADC_SAMPLE_FREQUENCY        8000.0
#define ADC_NUM_BITS                        32

using namespace std;

SC_MODULE(AudioCapture) {
    // AMS components
    microphone microphone0;
    filter_decoup filter0;
    adc_converter<ADC_NUM_BITS> adc_converter0;

    // AMS signals
    sca_tdf::sca_signal<double> microphone_out;
    sca_tdf::sca_signal<double> filter_out;
    sc_core::sc_signal<sc_dt::sc_int<ADC_NUM_BITS>> adc_out;

    // Events
    sc_event event_thread_process, done;

    // Queues
    queue<tlm::tlm_generic_payload*> trans_pending;

    // Sockets
    tlm_utils::simple_target_socket<AudioCapture> target_socket;
    tlm_utils::simple_initiator_socket<AudioCapture> initiator_socket;

    SC_CTOR(AudioCapture): target_socket("target_socket"), initiator_socket("initiator_socket"),
        microphone0("microphone", sc_core::sc_time((1.0/DEFAULT_MICROPHONE_SAMPLE_FREQUENCY), sc_core::SC_SEC)),
        filter0("filter", DEFAULT_FILTER_DEC_CUTOFF_FREQUENCY, DEFAULT_FILTER_DEC_GAIN),
        adc_converter0("adc_converter", (pow(2,ADC_NUM_BITS-1)-1), sc_core::sc_time((1.0/DEFAULT_ADC_SAMPLE_FREQUENCY), sc_core::SC_SEC)) {
        
        // AMS Connections
        microphone0.out(microphone_out);

        filter0.in(microphone_out);
        filter0.out(filter_out);

        adc_converter0.in(filter_out);
        adc_converter0.out(adc_out);

        // Sockets
        target_socket.register_nb_transport_fw(this, &AudioCapture::nb_transport_fw);
        initiator_socket.register_nb_transport_bw(this, &AudioCapture::nb_transport_bw);

        // Threads
        SC_THREAD(thread_process);
        SC_THREAD(io_request);
        SC_THREAD(thread_notify);
    }

    void io_request();
    void thread_process();
    void thread_notify();

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay );
};

#endif

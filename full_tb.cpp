#include "systemc.h"
#include "systemc-ams.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "AudioFile.h"
#include "lpc_analizer.h"
#include "lpc_synthesis.h"
#include "HammingEnc.h"
#include "HammingDec.h"
#include "AudioCapture.h"
#include "Channel.h"
#include "register_map.h"

#define ROUTER_TARGET_DECODER 0
#define ROUTER_TARGET_SYNTHESIS 1

using namespace std;

// *********************************************
// Router transmitter
// *********************************************
template<unsigned int N_TARGETS>
struct Router_t: sc_module {
    tlm_utils::simple_target_socket<Router_t>   target_socket_lpc;
    tlm_utils::simple_target_socket<Router_t>   target_socket_enc;
    tlm_utils::simple_target_socket<Router_t>   target_socket_audio;

    tlm_utils::simple_initiator_socket<Router_t> initiator_socket_lpc;
    tlm_utils::simple_initiator_socket<Router_t> initiator_socket_enc;
    tlm_utils::simple_initiator_socket<Router_t> initiator_socket_audio;
    tlm_utils::simple_initiator_socket<Router_t> initiator_socket_channel;


    SC_CTOR(Router_t):  target_socket_lpc("target_socket_lpc"), target_socket_enc("target_socket_enc"), target_socket_audio("target_socket_audio"),
                        initiator_socket_lpc("initiator_socket_lpc"), initiator_socket_audio("initiator_socket_audio"),
                        initiator_socket_enc("initiator_socket_enc"), initiator_socket_channel("initiator_socket_channel") {
        // Register callbacks for incoming interface method calls
        target_socket_lpc.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
        target_socket_enc.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
        target_socket_audio.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
        initiator_socket_enc.register_nb_transport_bw(this, &Router_t::nb_transport_bw);
        initiator_socket_channel.register_nb_transport_bw(this, &Router_t::nb_transport_bw);
        initiator_socket_audio.register_nb_transport_bw(this, &Router_t::nb_transport_bw);
        initiator_socket_lpc.register_nb_transport_bw(this, &Router_t::nb_transport_bw);
    }

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        sc_dt::uint64 addr = trans.get_address();

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        if (addr ==  ENCODER_COEFF) {
            status = ( initiator_socket_enc )->nb_transport_fw( trans, phase, delay );
        } else if (addr == ANALIZER_COEFF) {
            status = ( initiator_socket_lpc )->nb_transport_fw( trans, phase, delay );
        } else if (addr >= FILTER_CHANNEL_GAIN) {
            status = ( initiator_socket_channel )->nb_transport_fw( trans, phase, delay );
        } else {
            status = ( initiator_socket_audio )->nb_transport_fw( trans, phase, delay );
        }

        return status;
    }

    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        sc_dt::uint64 addr = trans.get_address();

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        if (addr == ENCODER_COEFF ) {
            status = ( target_socket_lpc )->nb_transport_bw( trans, phase, delay );
        } else if (addr == ANALIZER_COEFF) {
            status = ( target_socket_audio )->nb_transport_bw( trans, phase, delay );
        } else if (addr >= FILTER_CHANNEL_GAIN) {
            status = ( target_socket_enc )->nb_transport_bw( trans, phase, delay );
        }
        
        return status;
    }
};

// *********************************************
// Router receiver
// *********************************************
template<unsigned int N_TARGETS>
struct Router_r: sc_module {
    tlm_utils::simple_target_socket<Router_r>   target_socket_tra;
    tlm_utils::simple_target_socket<Router_r>   target_socket_dec;

    tlm_utils::simple_initiator_socket<Router_r> initiator_socket_dec;
    tlm_utils::simple_initiator_socket<Router_r> initiator_socket_syn;

    SC_CTOR(Router_r) : target_socket_tra("target_socket_tra"), target_socket_dec("target_socket_dec"), initiator_socket_dec("initiator_socket_dec"), initiator_socket_syn("initiator_socket_syn") {
        // Register callbacks for incoming interface method calls
        target_socket_tra.register_nb_transport_fw(this, &Router_r::nb_transport_fw);
        target_socket_dec.register_nb_transport_fw(this, &Router_r::nb_transport_fw);
        initiator_socket_dec.register_nb_transport_bw(this, &Router_r::nb_transport_bw);
        initiator_socket_syn.register_nb_transport_bw(this, &Router_r::nb_transport_bw);
    }

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        unsigned int target_nr = decode_address( trans.get_address() );

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        if (target_nr == 0 ) {
            status = ( initiator_socket_dec )->nb_transport_fw( trans, phase, delay );
        } else {
            status = ( initiator_socket_syn )->nb_transport_fw( trans, phase, delay );
        }
        
        return status;
    }

    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        unsigned int target_nr = decode_address( trans.get_address() );

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        if (target_nr == 0 ) {
            status = ( target_socket_tra )->nb_transport_bw( trans, phase, delay );
        } else {
            status = ( target_socket_dec )->nb_transport_bw( trans, phase, delay );
        }
        
        return status;
    }

    // Simple fixed address decoding
    inline unsigned int decode_address( sc_dt::uint64 addr ) {
        unsigned int target_nr;
        if (addr == DECODER_COEFF) {
            target_nr = ROUTER_TARGET_DECODER;
        } else {
            target_nr = ROUTER_TARGET_SYNTHESIS;
        }
        return target_nr;
    }
};

SC_MODULE(Top) {
    AudioCapture*  AudioCapture0;
    lpc_analizer*  lpc_analizer_i;
    Router_t<1>*   router_t;
    Router_r<1>*   router_r;
    HammingEnc*    encoder;
    HammingDec*    decoder;
    lpc_synthesis* lpc_synthesis_i;
    Channel*       Channel0;
    

    SC_CTOR(Top) {
        // Instantiate components
        AudioCapture0   = new AudioCapture("AUDIO_CAPTURE");
        lpc_analizer_i  = new lpc_analizer("LPC_ANALIZER");
        router_t        = new Router_t<1>("ROUTER_T");
        router_r        = new Router_r<1>("ROUTER_R");
        encoder         = new HammingEnc("HAMMING_ENC");
        decoder         = new HammingDec("HAMMING_DEC");
        lpc_synthesis_i = new lpc_synthesis("LPC_SYNTHESIS");
        Channel0        = new Channel("CHANNEL");

        /////////////////////////
        // Bind sockets

        // Audio capture
        AudioCapture0->initiator_socket.bind(router_t->target_socket_audio);

        // Analizer
        lpc_analizer_i->initiator_socket.bind( router_t->target_socket_lpc );

        // Encoder
        encoder->initiator_socket.bind( router_t->target_socket_enc );

        // Router
        router_t->initiator_socket_enc.bind( encoder->target_socket );
        router_t->initiator_socket_channel.bind( Channel0->target_socket );
        router_t->initiator_socket_audio.bind( AudioCapture0->target_socket );
        router_t->initiator_socket_lpc.bind( lpc_analizer_i->target_socket );
        router_r->initiator_socket_dec.bind( decoder->target_socket );
        router_r->initiator_socket_syn.bind( lpc_synthesis_i->socket );

        // Decoder
        decoder->initiator_socket.bind( router_r->target_socket_dec );

        // Channel
        Channel0->initiator_socket.bind( router_r->target_socket_tra );
    }
};

int sc_main(int argc, char* argv[])  {

    Top top("TOP");

    // Tracing
  	sca_util::sca_trace_file* atf = sca_util::sca_create_vcd_trace_file( "full.vcd" );
  	sca_util::sca_trace( atf, top.Channel0->prot_gen_out, "Channel_prot_gen_out" );
  	sca_util::sca_trace( atf, top.Channel0->mod_out, "Channel_mod_out" );
  	sca_util::sca_trace( atf, top.Channel0->demod0.rc.out, "Channel_rec_out" );
  	sca_util::sca_trace( atf, top.Channel0->demod0.lp.out, "Channel_filter_out" );
 	sca_util::sca_trace( atf, top.Channel0->demod_out, "Channel_demod_out" );
    sca_util::sca_trace(atf, top.AudioCapture0->microphone_out, "Audio_microphone_out");
    sca_util::sca_trace(atf, top.AudioCapture0->filter_out, "Audio_filter_out");
    sca_util::sca_trace(atf, top.AudioCapture0->adc_out, "Audio_adc_out");

    sc_start();

    sc_start(1000,SC_NS);

    sca_util::sca_close_vcd_trace_file( atf );

    return 0;
}

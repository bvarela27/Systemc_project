#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "AudioFile.h"
#include "lpc_analizer.h"
#include "lpc_synthesis.h"
#include "HammingEnc.h"
#include "register_map.h"

#define ROUTER_TARGET_ENCODER 0
#define ROUTER_TARGET_RECEIVER 1

using namespace std;

// *********************************************
// Router transmitter
// *********************************************
template<unsigned int N_TARGETS>
struct Router_t: sc_module {
    tlm_utils::simple_target_socket<Router_t>   target_socket_lpc;
    tlm_utils::simple_target_socket<Router_t>   target_socket_enc;

    tlm_utils::simple_initiator_socket<Router_t> initiator_socket_enc;
    tlm_utils::simple_initiator_socket<Router_t> initiator_socket_rec;

    SC_CTOR(Router_t) : target_socket_lpc("target_socket_lpc"), target_socket_enc("target_socket_enc"), initiator_socket_enc("initiator_socket_enc"), initiator_socket_rec("initiator_socket_rec") {
        // Register callbacks for incoming interface method calls
        target_socket_lpc.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
        target_socket_enc.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
        initiator_socket_enc.register_nb_transport_bw(this, &Router_t::nb_transport_bw);
        initiator_socket_rec.register_nb_transport_bw(this, &Router_t::nb_transport_bw);
    }

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        unsigned int target_nr = decode_address( trans.get_address() );

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        if (target_nr == 0 ) {
            status = ( initiator_socket_enc )->nb_transport_fw( trans, phase, delay );
        } else {
            status = ( initiator_socket_rec )->nb_transport_fw( trans, phase, delay );
        }
        
        return status;
    }

    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        unsigned int target_nr = decode_address( trans.get_address() );

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        if (target_nr == 0 ) {
            status = ( target_socket_lpc )->nb_transport_bw( trans, phase, delay );
        } else {
            status = ( target_socket_enc )->nb_transport_bw( trans, phase, delay );
        }
        
        return status;
    }

    // Simple fixed address decoding
    inline unsigned int decode_address( sc_dt::uint64 addr ) {
        unsigned int target_nr;
        // FIXME use decoder instead of synthesis
        if (addr == ENCODER_COEFF) {
            target_nr = ROUTER_TARGET_ENCODER;
        } else {
            target_nr = ROUTER_TARGET_RECEIVER;
        }
        return target_nr;
    }
};

SC_MODULE(Top) {
    lpc_analizer*  lpc_analizer_i;
    Router_t<1>*   router_t;
    HammingEnc*    encoder;
    lpc_synthesis* lpc_synthesis_i;
    

    SC_CTOR(Top) {
        // Instantiate components
        lpc_analizer_i  = new lpc_analizer("LPC_ANALIZER");
        router_t        = new Router_t<1>("ROUTER_T");
        encoder         = new HammingEnc("HAMMING_ENC");
        lpc_synthesis_i = new lpc_synthesis("LPC_SYNTHESIS");

        /////////////////////////
        // Bind sockets

        // Analizer
        lpc_analizer_i->socket.bind( router_t->target_socket_lpc );

        // Router
        router_t->initiator_socket_enc.bind( encoder->target_socket );
        router_t->initiator_socket_rec.bind( lpc_synthesis_i->socket );

        // Encoder
        encoder->initiator_socket.bind( router_t->target_socket_enc );
    }
};

int sc_main(int argc, char* argv[])  {

    Top top("TOP");
    
    sc_start();

    ////////////////////////////////////////////////////
    // Read WAV file
    AudioFile<double> audioFile;
    audioFile.load("speech.wav");

    int channel = 0;
    int sample_rate = 8000;
    vector<double> samples = audioFile.samples[channel];

    ////////////////////////////////////////////////////
    // Set samples and rate
    top.lpc_analizer_i->set_samples(samples, sample_rate);

    sc_start(100,SC_NS);

    return 0;
}

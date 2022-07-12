#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "AudioFile.h"
#include "lpc_analizer.h"
#include "lpc_synthesis.h"
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

    tlm_utils::simple_initiator_socket_tagged<Router_t>* initiator_socket[N_TARGETS];

    SC_CTOR(Router_t) : target_socket_lpc("target_socket_lpc"), target_socket_enc("target_socket_enc")  {
        // Register callbacks for incoming interface method calls
        target_socket_lpc.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
        target_socket_enc.register_nb_transport_fw(this, &Router_t::nb_transport_fw);
   
        for (unsigned int i = 0; i < N_TARGETS; i++) {
            char txt[20];
            sprintf(txt, "socket_%d", i);
            initiator_socket[i] = new tlm_utils::simple_initiator_socket_tagged<Router_t>(txt);
        }
    }

    // TLM-2 blocking transport method
    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        unsigned int target_nr = decode_address( trans.get_address() );

        // Forward transaction to appropriate target
        tlm::tlm_sync_enum status;
        status = ( *initiator_socket[target_nr] )->nb_transport_fw( trans, phase, delay );
        
        return status;
    }

    // Simple fixed address decoding
    inline unsigned int decode_address( sc_dt::uint64 addr ) {
        unsigned int target_nr;
        // FIXME use decoder instead of synthesis
        if (addr >= SYN_POLO_0) {
            target_nr = ROUTER_TARGET_RECEIVER;
        } else {
            target_nr = ROUTER_TARGET_ENCODER;
        }
        return target_nr;
    }
};

struct Encoder: sc_module {

    sc_event event;
    tlm_utils::simple_target_socket<Encoder> target_socket;
    tlm_utils::simple_initiator_socket<Encoder> initiator_socket;

    SC_CTOR(Encoder): target_socket("target_socket"), initiator_socket("initiator_socket") {
        // Register callbacks for incoming interface method calls
        target_socket.register_nb_transport_fw(this, &Encoder::nb_transport_fw);
        SC_THREAD(thread_process);
    }

    void thread_process() {
        while (true) {
        	wait(event);

        	tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            tlm::tlm_sync_enum status;
        	sc_dt::uint64 addr;
            double data;
        	sc_time delay = sc_time(1, SC_NS);

        	trans->set_command( tlm::TLM_WRITE_COMMAND );
        	trans->set_data_length( 8 );
        	trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
        	trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
        	trans->set_dmi_allowed( false ); // Mandatory initial value
        	trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

        	for (int i=0; i<LPC_ORDER+2; i++) {
        	    cout << name() << " BEGIN_REQ SENT" << " at time " << sc_time_stamp() << endl;

        	    addr = SYN_POLO_0 + 0x00000008*i;
                data = (double) i ;

        	    trans->set_address( addr );
        	    trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data) );

        	    status = initiator_socket->nb_transport_fw( *trans, phase, delay );  // Blocking transport call

                switch (status) {
                    case tlm::TLM_ACCEPTED:
                        cout << name() << " NB_TRANSPORT_FW (STATUS TLM_ACCEPTED)" << " at time " << sc_time_stamp() << endl;
                        break;
                    default:
                        cout << name() << " NB_TRANSPORT_FW (STATUS not expected)" << " at time " << sc_time_stamp() << endl;
                        break;
                }

        	    // Initiator obliged to check response status
        	    if ( trans->is_response_error() ) {
        	        char txt[100];
        	        sprintf(txt, "Error from b_transport, response status = %s", trans->get_response_string().c_str());
        	        SC_REPORT_ERROR("TLM-2", txt);
        	    }
        	}
        } 
    }

    // Blocking transport method
    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64    addr = trans.get_address();
        unsigned char*   ptr = trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        wait(delay);

        double data;
        memcpy(&data, ptr, len);

        //cout << name() << " Data received: " << data << endl;
        //cout << name() << " BEGIN_REQ RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
        
        if (phase == tlm::BEGIN_REQ) {
            cout << name() << " BEGIN_REQ RECEIVED" << " at time " << sc_time_stamp() << endl;
        }

        // FIXME ENC_PITCH instead
        if (addr == ENC_GAIN) {
            event.notify(0, SC_NS);
        }

        // Obliged to set response status to indicate successful completion
        trans.set_response_status( tlm::TLM_OK_RESPONSE );

        return tlm::TLM_ACCEPTED;
    }
};

SC_MODULE(Top) {
    lpc_analizer*  lpc_analizer_i;
    Router_t<2>*   router_t;
    Encoder*       encoder;
    lpc_synthesis* lpc_synthesis_i;
    

    SC_CTOR(Top) {
        // Instantiate components
        lpc_analizer_i  = new lpc_analizer("lpc_analizer");
        router_t        = new Router_t<2>("router_t");
        encoder         = new Encoder("encoder");
        lpc_synthesis_i = new lpc_synthesis("lpc_synthesis");

        /////////////////////////
        // Bind sockets

        // Analizer
        lpc_analizer_i->socket.bind( router_t->target_socket_lpc );

        // Router
        router_t->initiator_socket[ROUTER_TARGET_ENCODER]->bind( encoder->target_socket );
        router_t->initiator_socket[ROUTER_TARGET_RECEIVER]->bind( lpc_synthesis_i->socket );

        // Encoder
        encoder->initiator_socket.bind( router_t->target_socket_enc );
    }
};

int sc_main(int argc, char* argv[])  {

    Top top("top");
    
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

    sc_start(50,SC_NS);

    return 0;
}

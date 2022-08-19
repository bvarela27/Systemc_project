#include "systemc.h"
#include <iostream>
#include <math.h>
#include "lpc_analizer.h"
#include "tlm.h"
#include "register_map.h"

using namespace std;

// User-defined extension class
struct ID_extension: tlm::tlm_extension<ID_extension> {
  ID_extension() : transaction_id(0) {}
  virtual tlm_extension_base* clone() const { // Must override pure virtual clone method
    ID_extension* t = new ID_extension;
    t->transaction_id = this->transaction_id;
    return t;
  }

  // Must override pure virtual copy_from method
  virtual void copy_from(tlm_extension_base const &ext) {
    transaction_id = static_cast<ID_extension const &>(ext).transaction_id;
  }
  unsigned int transaction_id;
};

tlm::tlm_sync_enum lpc_analizer::nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension );

    if (phase == tlm::BEGIN_REQ) {
        // Check len
        trans_pending.push(&trans);

        // Trigger event
        event_thread_process.notify();

        // Delay
        wait(delay);

        // Display message
        cout << name() << "   BEGIN_REQ RECEIVED " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

        // Obliged to set response status to indicate successful completion
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    } else {
        cout << name() << " NB_TRANSPORT_FW unexpected phase" << " at time " << sc_time_stamp() << endl;
        trans.set_response_status( tlm::TLM_GENERIC_ERROR_RESPONSE );
    }
    return tlm::TLM_ACCEPTED;
};

void lpc_analizer::thread_process_resp() {
    while (true) {
        // Variables
        tlm::tlm_phase phase_bw = tlm::BEGIN_RESP;
        tlm::tlm_sync_enum status_bw;
        sc_time delay_bw;
        int32_t data;
        ID_extension* id_extension = new ID_extension;
        tlm::tlm_generic_payload * current_trans;

        wait(event_thread_process);

        current_trans = trans_pending.front();
        trans_pending.pop();
        unsigned char*   ptr = current_trans->get_data_ptr();
        current_trans->get_extension( id_extension );
        data = *reinterpret_cast<uint32_t*>(ptr);

        // Set sample
        set_sample(data);

        // Obliged to set response status to indicate successful completion
        current_trans->set_response_status( tlm::TLM_OK_RESPONSE );

        cout << name() << "   BEGIN_RESP SENT" << "    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

        // Backward call
        delay_bw= sc_time(1, SC_NS);
        status_bw = target_socket->nb_transport_bw( *current_trans, phase_bw, delay_bw );

        switch (status_bw) {
            case tlm::TLM_ACCEPTED:
                cout << name() << "   NB_TRANSPORT_BW (STATUS TLM_ACCEPTED)" << " at time " << sc_time_stamp() << endl;
                break;
            default:
                cout << name() << "   NB_TRANSPORT_BW (STATUS not expected)" << " at time " << sc_time_stamp() << endl;
                break;
        }
    }
};

// Thread process (Initiator)
void lpc_analizer::thread_process() {
    // Local variables
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status;
    sc_time delay = sc_time(5, SC_NS);

    uint32_t window_counter = 0;
    while (true) {
        // Wait the user to set the samples
        wait(start_initiator);

        int32_t trans_data[WINDOW+2];

        compute_LPC_window(trans_data);

        // Common fields
        for (uint32_t i=0; i<LPC_ORDER+2; i++) {
            tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;
            ID_extension* id_extension = new ID_extension;
            id_extension->transaction_id = i+(LPC_ORDER+2)*window_counter;
            trans->set_command( tlm::TLM_WRITE_COMMAND );
            trans->set_data_length( 8 );
            trans->set_address( ENCODER_COEFF );
            trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
            trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
            trans->set_dmi_allowed( false ); // Mandatory initial value
            trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
            trans->set_extension(id_extension);

            cout << name() << "  BEGIN_REQ SENT " << "    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

            trans->set_data_ptr( reinterpret_cast<unsigned char*>(&trans_data[i]));

            status = initiator_socket->nb_transport_fw(*trans, phase, delay );

            switch (status) {
                case tlm::TLM_ACCEPTED:
                    cout << name() << "  NB_TRANSPORT_FW (STATUS TLM_ACCEPTED)" << " at time " << sc_time_stamp() << endl;
                    break;
                default:
                    cout << name() << "  NB_TRANSPORT_FW (STATUS not expected)" << " at time " << sc_time_stamp() << endl;
                    break;
            }

            // Initiator obliged to check response status
            if ( trans->is_response_error() ) {
                char txt[100];
                sprintf(txt, "Error from b_transport, response status = %s", trans->get_response_string().c_str());
                SC_REPORT_ERROR("TLM-2", txt);
            }
        }
        window_counter++;
    }
    while(true) {
        wait(1,SC_NS);
    }
};

// Backward path non-blocking (Initiator)
tlm::tlm_sync_enum lpc_analizer::nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {

    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension );

    if (phase == tlm::BEGIN_RESP) {

        // Initiator obliged to check response status   
        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport");
        }

        //Delay
        wait(delay);

        cout << name () << "  BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
    }

    return tlm::TLM_ACCEPTED;
};

// Set sample
void lpc_analizer::set_sample(double sample) {
    samples[sample_number] = sample;
    sample_number++;

    if (sample_number == WINDOW) {
        // Normalize samples
        normalize_samples();

        // OLA
        set_OLA();

        // Move last 120 samples to the beginnig of the array (overlap)
        for (int i=0; i<WINDOW/2; i++) {
            samples[i] = samples[WINDOW/2+i];
        }

        // Restart counter from 120
        sample_number = WINDOW/2;

        // Notify initiator to send new coeffs
        start_initiator.notify(0, SC_NS);
    }
};

// Normalize function
void lpc_analizer::normalize_samples() {
    // Samples abs values
    double samples_abs[WINDOW];

    for (int i=0; i<WINDOW; i++) {
        samples_abs[i] = abs(samples[i]);
    }

    // Get max value
    double max_sample_value = *max_element(samples_abs, samples_abs+WINDOW);
    
    // Normalize samples
    for (int i=0; i<WINDOW; i++) {
        samples_normalized[i] = 0.9*samples[i]/max_sample_value;
    }
};

// OLA
void lpc_analizer::set_OLA() {
    double hann_window[WINDOW];

    hanning(hann_window,WINDOW);

    for(int i = 0; i < WINDOW; i++) {
        stacked[i] = hann_window[i]*samples_normalized[i];
    }
};

// LPC Analizer
void lpc_analizer::compute_LPC(double* poles_gain) {
    double A[WINDOW-1][N_POLES] = {0};
    double b[WINDOW-1] = {0};
    double poles[N] = {0};
    double U[M][N];
    double V[N][N];
    double Dum[N];
    double mul_A_a[M];
    double error[N];
    double S[N];
    double x[N];

    memcpy(b,stacked+1,(WINDOW-1)*sizeof(double));

    for(int j = 0; j<N_POLES; j++) {
        for(int i = j; i < (WINDOW-j-2); i++) {
            A[i][j] = stacked[i-j];
        }
    }

    int err = Singular_Value_Decomposition((double *)A,M,N,(double *)U,S,(double*)V,Dum);
    Singular_Value_Decomposition_Solve((double *)U,S,(double *)V,0,M,N,b,poles);

    Multiply_Matrices(mul_A_a,(double *)A,M,N,poles,1);
    Subtract_Matrices(error,b,mul_A_a,M,1);

    double G = variance(error,M);

    // Fill new array with poles and gain
    for (int i=0; i<N; i++) {
        poles_gain[i] = poles[i];
    }
    poles_gain[N] = G;

    return;
};

void lpc_analizer::compute_LPC_window(int32_t* poles_gain_pitch) {
    bool valid = 1;
    double gain = 0;
    double pitch = 0;
    double poles_gain[N_POLES+1];

    compute_LPC(poles_gain);

    // Pitch
    double f[N_FFT];
    frequency(f, 1);
    pitch = find_pitch( stacked, f, WINDOW);

    for (int i=0; i<N_POLES+1; i++) {
        if (i == N_POLES) { // Gain
            poles_gain_pitch[i] = DoubleToFixed(poles_gain[i]*1000);
        } else {            // Coeff
            poles_gain_pitch[i] = DoubleToFixed(poles_gain[i]);
        }
    }
    // Pitch
    poles_gain_pitch[N_POLES+1] = DoubleToFixed(pitch);

    current_window++;

    return;
};

#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "hanning.h"
#include "lpc_analizer.h"
#include "tlm.h"
#include "register_map.h"
#include "pitch_detection.h"
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

// Thread process (Initiator)
void lpc_analizer::thread_process() {
    // Local variables
    bool valid;
    int32_t gain;
    int32_t pitch;
    vector<int32_t> coeffs;

    // Wait the user to set the samples
    wait(start_initiator);
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status;
    sc_time delay = sc_time(5, SC_NS);

    vector<int32_t> trans_data;
    // Generate a random sequence of reads and writes
    uint32_t window_counter = 0;
    while (true) {
        tie(valid, gain, pitch, coeffs) = compute_LPC_window();

        trans_data.resize((coeffs.size()+2)*(1+window_counter));
        copy(coeffs.begin(), coeffs.end(),trans_data.begin() + (coeffs.size()+2)*window_counter);
        trans_data[(LPC_ORDER) + (LPC_ORDER+2)*window_counter] = gain;
        trans_data[(LPC_ORDER+1) + (LPC_ORDER+2)*window_counter] = pitch;

        coeffs.clear();

        if (valid && (window_counter<2)) {
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

                trans->set_data_ptr( reinterpret_cast<unsigned char*>(&trans_data[i+26*window_counter]));

                status = socket->nb_transport_fw(*trans, phase, delay );

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


        } else {
        break;
        }
        window_counter++;
    }
    while(true){
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


// Set samples and sample_rate
void lpc_analizer::set_samples(vector<double> samples_arg, double sample_rate_arg = 44100) {
    samples = samples_arg;
    sample_rate = sample_rate_arg;

    // Set pointer to the first window
    current_window = 0;

    normalize_samples();

    set_OLA();

    // Notify initiator
    start_initiator.notify(0, SC_NS);
};

// Normalize function
void lpc_analizer::normalize_samples() {
    // Samples abs values
    vector<double> samples_abs;
    for (int i=0; i<samples.size(); i++) {
        samples_abs.push_back(abs(samples[i]));
    }

    // Get max value
    double max_sample_value = *max_element(samples_abs.begin(), samples_abs.end());
    
    // Normalize samples
    for (int i=0; i<samples.size(); i++) {
        samples[i] = 0.9*samples[i]/max_sample_value;
    }
};

// Hann window 30ms
double lpc_analizer::get_window_points () {
    return floor(WINDOW_SIZE_IN_S*sample_rate);
};

double* lpc_analizer::get_hann_window (double window_points) {
    return hanning(window_points, 1);
};

// OLA
void lpc_analizer::set_OLA() {
    double window_points = get_window_points();
    double* hann_window = get_hann_window(window_points);

    double overlap_size = floor(window_points*(double)(OVERLAP_PERCENTAGE)/(double)(100));
    double num_windows = ((samples.size()-window_points) > 0) ? ceil((samples.size()-window_points)/overlap_size) + 1 : 1;

    arma::Mat<double> OLA_int(window_points, num_windows, arma::fill::zeros);

    for (int j=0; j<num_windows; j++) {
        for (int i=0; i<window_points; i++) {
            if ((j)*overlap_size+i < samples.size()) {
                OLA_int(i,j) = hann_window[i] * samples[(j)*overlap_size+i];
            }
        }
    }

    OLA = OLA_int;
};

// LPC Analizer
tuple <arma::Mat<double>, double> lpc_analizer::compute_LPC(arma::Mat<double> samples, int p) {

    int N = samples.size();
    arma::Mat<double> b (N-1, 1);

    for (int i=0; i<N-1; i++) {
        b(i, 0) = samples(i+1, 0);
    }

    arma::Mat<double> temp;
    arma::Mat<double> A (N-1, p, arma::fill::zeros);

    // Create autocorrelation matrix
    for (int i=0; i<p; i++) {
        temp = shift(samples, i);
        for (int j=0; j<N-1; j++) {
            A(j, i)  = temp(j, 0);
        }
    }

    // Solve Levinson-Gurbin recursion
    arma::Mat<double> a = solve(A, b);

    // Calculate the errors errors
    arma::Mat<double> error = b - A*a;

    // Calculate variance of errors
    arma::Mat<double> g = var(error);

    return make_tuple(a, (double)g(0,0));
};

tuple <bool, int32_t, int32_t, vector<int32_t>> lpc_analizer::compute_LPC_window() {
    bool valid = 0;
    double gain = 0;
    double pitch = 0;
    vector<int32_t> coeffs;
    arma::Mat<double> coeffs_matrix;
    // There are NOT windows pending to be processed
    if (current_window >= OLA.n_cols) {
        valid = 0;
    // There are windows pending to be processed
    } else {
        valid = 1;
        tie(coeffs_matrix, gain) = compute_LPC(OLA.col(current_window), LPC_ORDER);
        //PITCH
        vec test = OLA.col(current_window);
        pitch = get_Pitch(test);
        for (int i=0; i<coeffs_matrix.n_rows; i++) {
            coeffs.push_back(DoubleToFixed(coeffs_matrix(i, 0)));
        }
        current_window++;
    }


    return make_tuple(valid, DoubleToFixed(gain*1000) , DoubleToFixed(pitch) , coeffs); //ADD PITCH
};

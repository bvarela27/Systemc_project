#include "systemc.h"
#include <iostream>
#include <vector>
#include <armadillo>
#include <math.h>
#include "hanning.h"
#include "lpc_analizer.h"
#include "tlm.h"
#include "register_map.h"

using namespace std;

////////////////////////////////////////////////////
// Thread process (Initiator)
void lpc_analizer::thread_process() {
    // Local variables
    bool valid;
    double gain;
    vector<double> coeffs;

    // Wait the user to set the samples
    wait(start_initiator);

    tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status;
    sc_time delay = sc_time(1, SC_NS);

    // Generate a random sequence of reads and writes
    while (true) {
        tie(valid, gain, coeffs) = compute_LPC_window();
        if (valid) {
            // Common fields
            tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
            trans->set_command( tlm::TLM_WRITE_COMMAND );
            trans->set_data_length( 8 );
            trans->set_address( ENCODER_COEFF );
            trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
            trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
            trans->set_dmi_allowed( false ); // Mandatory initial value
            trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

            for (int i=0; i<LPC_ORDER; i++) {
                //cout << "LPC_ANALIZER BEGIN_REQ SENT" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
                cout << name() << " BEGIN_REQ SENT" << " at time " << sc_time_stamp() << " Coeff[" << i << "]: " << coeffs[i] << endl;

                trans->set_data_ptr( reinterpret_cast<unsigned char*>(&coeffs[i]) );

                status = socket->nb_transport_fw( *trans, phase, delay );

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
        } else {
            break;
        }
    }
};

////////////////////////////////////////////////////
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

////////////////////////////////////////////////////
// Backward path non-blocking (Initiator)
tlm::tlm_sync_enum lpc_analizer::nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();

    //ID_extension* id_extension = new ID_extension;
    //trans.get_extension( id_extension );

    if (phase == tlm::BEGIN_RESP) {

        // Initiator obliged to check response status   
        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport");
        }

        //cout << "trans/bw = { " << (cmd ? 'W' : 'R') << ", " << hex << adr
        //     << " } , data = " << hex << data << " at time " << sc_time_stamp()
        //     << ", delay = " << delay << endl;

        cout << "trans/bw = { " << (cmd ? 'W' : 'R') << ", " << hex << adr
             << " } ," << " at time " << sc_time_stamp()
             << ", delay = " << delay << endl;

        //Delay
        wait(delay);

        //cout << name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
        cout << name() << " BEGIN_RESP RECEIVED" << " at time " << sc_time_stamp() << endl;
    }

    return tlm::TLM_ACCEPTED;
};


////////////////////////////////////////////////////
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

////////////////////////////////////////////////////
// Hann window 30ms
double lpc_analizer::get_window_points () {
    return floor(WINDOW_SIZE_IN_S*sample_rate);
};

double* lpc_analizer::get_hann_window (double window_points) {
    return hanning(window_points, 1);
};

////////////////////////////////////////////////////
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

////////////////////////////////////////////////////
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

tuple <bool, double, vector<double>> lpc_analizer::compute_LPC_window() {
    bool valid;
    double gain;
    vector<double> coeffs;
    arma::Mat<double> coeffs_matrix;

    // There are NOT windows pending to be processed
    if (current_window >= OLA.n_cols) {
        valid = 0;
    // There are windows pending to be processed
    } else {
        valid = 1;
        tie(coeffs_matrix, gain) = compute_LPC(OLA.col(current_window), LPC_ORDER);
        for (int i=0; i<coeffs_matrix.n_rows; i++) {
            coeffs.push_back((double)coeffs_matrix(i, 0));
        }
        current_window++;
    }

    return make_tuple(valid, gain, coeffs);
};
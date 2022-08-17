#include <iostream>
#include <systemc.h>
#include <stdlib.h>
#include "AudioCapture.h"

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

tlm::tlm_sync_enum AudioCapture::nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
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

void AudioCapture::thread_notify() {
    while (true) {
        wait(done);

        // Wait thread_process to start over
        wait(1, SC_NS);

        if (!trans_pending.empty()) {
            // Trigger event
            event_thread_process.notify();
        }
    }
};

void AudioCapture::thread_process() {
    tlm::tlm_phase phase_bw = tlm::BEGIN_RESP;
    tlm::tlm_sync_enum status_bw;
    sc_time delay_bw;
    int32_t data;
    ID_extension* id_extension = new ID_extension;
    tlm::tlm_generic_payload * current_trans;

    while (true) {
        wait(event_thread_process);

        // Update register value
        current_trans = trans_pending.front();
        trans_pending.pop();
        unsigned char*   ptr  = current_trans->get_data_ptr();
        unsigned int     len  = current_trans->get_data_length();
        sc_dt::uint64    addr = current_trans->get_address();
        current_trans->get_extension( id_extension );
        data = *reinterpret_cast<uint32_t*>(ptr);

        switch (addr) {
            case FILTER_AUDIO_GAIN:
                filter0.set_gain(data);
                cout << name() << " Register Filter Gain updated" << " at time " << sc_time_stamp() << endl;
                break;
            case FILTER_AUDIO_CUTOFF_FREQUENCY:
                filter0.set_cutoff_frequency(data);
                cout << name() << " Register Filter Cutoff Frequency updated" << " at time " << sc_time_stamp() << endl;
                break;
            case ADC_SAMPLE_FREQUENCY:
                adc_converter0.set_sample_frequency(data);
                cout << name() << " Register ADC Sample Frequency updated" << " at time " << sc_time_stamp() << endl;
                break;
            default:
                SC_REPORT_ERROR("TLM-2", "ERROR: Unexpected address received in the AudioCapture module");
                break;
        }

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

        // Initiator obliged to check response status
        if ( current_trans->is_response_error() ) {
            char txt[100];
            sprintf(txt, "Error from b_transport, response status = %s", current_trans->get_response_string().c_str());
            SC_REPORT_ERROR("TLM-2", txt);
        }
        done.notify();
    }
};

tlm::tlm_sync_enum AudioCapture::nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension );

    if (phase == tlm::BEGIN_RESP) {

        // Initiator obliged to check response status
        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport");
        }

        //Delay
        wait(delay);

        cout << name () << "   BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
    }

    return tlm::TLM_ACCEPTED;
};

void AudioCapture::io_request() {
    tlm::tlm_phase phase_fw = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status_fw;
    sc_time delay_fw;
    ID_extension* id_extension = new ID_extension;
    tlm::tlm_generic_payload * current_trans;

    int id = 0;

    while (true) {
        wait(adc_converter0.io_request);

        // Forward call Decoder
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
        ID_extension* id_extension = new ID_extension;
        id_extension->transaction_id = id;
        delay_fw = sc_time(2, SC_NS);

        // ADC_OUT casting
        // Note: If this is not added the value received in the other module doesn't make sense
        sc_dt::sc_int<32> adc_out_static_cast = static_cast<sc_dt::sc_int<32>>(adc_out);
        int adc_out_int = (int) adc_out_static_cast;

        trans->set_command( tlm::TLM_WRITE_COMMAND );
        trans->set_address( DECODER_COEFF );
        trans->set_data_length( 8 );
        trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
        trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
        trans->set_dmi_allowed( false ); // Mandatory initial value
        trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
        trans->set_extension( id_extension );
        trans->set_data_ptr( reinterpret_cast<unsigned char*>(&adc_out_int) );

        cout << name() << "   BEGIN_REQ SENT " << "    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

        status_fw = initiator_socket->nb_transport_fw( *trans, phase_fw, delay_fw );  // Blocking transport call

        switch (status_fw) {
            case tlm::TLM_ACCEPTED:
                cout << name() << "   NB_TRANSPORT_FW (STATUS TLM_ACCEPTED)" << " at time " << sc_time_stamp() << endl;
                break;
            default:
                cout << name() << "   NB_TRANSPORT_FW (STATUS not expected)" << " at time " << sc_time_stamp() << endl;
                break;
        }

        // Initiator obliged to check response status
        if ( trans->is_response_error() ) {
            char txt[100];
            sprintf(txt, "Error from b_transport, response status = %s", trans->get_response_string().c_str());
            SC_REPORT_ERROR("TLM-2", txt);
        }

        id++;
    }
};
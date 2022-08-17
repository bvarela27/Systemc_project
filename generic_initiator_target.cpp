#include <iostream>
#include "systemc.h"
#include <stdlib.h>
#include "generic_initiator_target.h"

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

tlm::tlm_sync_enum generic_initiator_target::nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
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

void generic_initiator_target::thread_process() {
    tlm::tlm_phase phase_bw = tlm::BEGIN_RESP;
    tlm::tlm_phase phase_fw = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status_bw, status_fw;
    sc_time delay_bw, delay_fw;
    int32_t data, data1;
    uint32_t encoded_data;
    ID_extension* id_extension = new ID_extension;
    tlm::tlm_generic_payload * current_trans;

    while (true) {

        wait(event_thread_process);
        // Execute Encoder with the information received
        current_trans = trans_pending.front();
        trans_pending.pop();
        unsigned char*   ptr = current_trans->get_data_ptr();
        current_trans->get_extension( id_extension );
        data = *reinterpret_cast<uint32_t*>(ptr);
        data1 = 4000;

        cout << "DATA RECEIVED: " << data << " at time " << sc_time_stamp() << endl;

        /*wait(sc_time(500000, SC_NS));
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
        delay_fw = sc_time(2, SC_NS);

        trans->set_command( tlm::TLM_WRITE_COMMAND );
        trans->set_address( ADC_SAMPLE_FREQUENCY );
        trans->set_data_length( 8 );
        trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
        trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
        trans->set_dmi_allowed( false ); // Mandatory initial value
        trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value
        trans->set_extension( id_extension );
        trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data1) );

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
        if ( trans->is_response_error() ) {
            char txt[100];
            sprintf(txt, "Error from b_transport, response status = %s", trans->get_response_string().c_str());
            SC_REPORT_ERROR("TLM-2", txt);
        }*/
    }
};

tlm::tlm_sync_enum generic_initiator_target::nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
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

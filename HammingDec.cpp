#include <iostream>
#include <systemc.h>
#include <stdlib.h>
#include "HammingDec.h"

void HammingDec::decode() {
    while(true) {
        wait(dec_t);
        reg = (sc_bv<32>) in.read();
        syn[0] = reg[0] ^reg[2] ^reg[4] ^reg[6] ^reg[8] ^reg[10]^reg[12]^reg[14]^reg[16]^reg[18]^reg[20]^reg[22]^reg[24]^reg[26]^reg[28]^reg[30];
        syn[1] = reg[1] ^reg[2] ^reg[5] ^reg[6] ^reg[9] ^reg[10]^reg[13]^reg[14]^reg[17]^reg[18]^reg[21]^reg[22]^reg[25]^reg[26]^reg[29]^reg[30];
        syn[2] = reg[3] ^reg[4] ^reg[5] ^reg[6] ^reg[11]^reg[12]^reg[13]^reg[14]^reg[19]^reg[20]^reg[21]^reg[22]^reg[27]^reg[28]^reg[29]^reg[30];
        syn[3] = reg[7] ^reg[8] ^reg[9] ^reg[10]^reg[11]^reg[12]^reg[13]^reg[14]^reg[23]^reg[24]^reg[25]^reg[26]^reg[27]^reg[28]^reg[29]^reg[30];
        syn[4] = reg[15]^reg[16]^reg[17]^reg[18]^reg[19]^reg[20]^reg[21]^reg[22]^reg[23]^reg[24]^reg[25]^reg[26]^reg[27]^reg[28]^reg[29]^reg[30];
        ecc_t.notify(6,SC_NS);
    }
}

void HammingDec::ECC(){
    while(true) {
        wait(ecc_t);
        sc_uint<32> syn_b =(sc_uint<32>)syn;
        if ( syn_b > 0 ) {
            reg[syn_b-1] = ~reg[syn_b-1];
        }

        decoded_val[0]  = reg[2];
        decoded_val[1]  = reg[4];
        decoded_val[2]  = reg[5];
        decoded_val[3]  = reg[6];
        decoded_val[4]  = reg[8];
        decoded_val[5]  = reg[9];
        decoded_val[6]  = reg[10];
        decoded_val[7]  = reg[11];
        decoded_val[8]  = reg[12];
        decoded_val[9]  = reg[13];
        decoded_val[10] = reg[14];
        decoded_val[11] = reg[16];
        decoded_val[12] = reg[17];
        decoded_val[13] = reg[18];
        decoded_val[14] = reg[19];
        decoded_val[15] = reg[20];
        decoded_val[16] = reg[21];
        decoded_val[17] = reg[22];
        decoded_val[18] = reg[23];
        decoded_val[19] = reg[24];
        decoded_val[20] = reg[25];
        decoded_val[21] = reg[26];
        decoded_val[22] = reg[27];
        decoded_val[23] = reg[28];
        decoded_val[24] = reg[29];
        decoded_val[25] = reg[30];
        decoded_val[26] = reg[31];
        decoded_val[27] = 0;
        decoded_val[28] = 0;
        decoded_val[29] = 0;
        decoded_val[30] = 0;
        decoded_val[31] = 0;

        out.write(decoded_val);
    }
}

void HammingDec::write() {
   dec_t.notify(2, SC_NS);
}

tlm::tlm_sync_enum HammingDec::nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    addr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    if (phase == tlm::BEGIN_REQ) {
        // Check len

        // Queue transaction
        queue_trans_pending.push_front(&trans);

        // Trigger event
        event_thread_process.notify();

        // Delay
        wait(delay);

        // Display message
        cout << name() << " BEGIN_REQ RECEIVED" << " at time " << sc_time_stamp() << endl;
        //cout << name() << " BEGIN_REQ RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

        // Obliged to set response status to indicate successful completion
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    } else {
        cout << name() << " NB_TRANSPORT_FW unexpected phase" << " at time " << sc_time_stamp() << endl;
        trans.set_response_status( tlm::TLM_GENERIC_ERROR_RESPONSE );
    }

    return tlm::TLM_ACCEPTED;
};

void HammingDec::thread_process() {
    tlm::tlm_phase phase_bw = tlm::BEGIN_RESP;
    tlm::tlm_phase phase_fw = tlm::BEGIN_REQ;

    tlm::tlm_sync_enum status_bw, status_fw;
    sc_time delay_bw, delay_fw;
    double data;
    uint32_t decoded_data;

    while (true) {
        wait(event_thread_process);

        // Execute Encoder with the information received
        tlm::tlm_generic_payload* trans_pending = queue_trans_pending.back();
        queue_trans_pending.pop_back();

        unsigned char*   ptr = trans_pending->get_data_ptr();
        unsigned int     len = trans_pending->get_data_length();

        memcpy(&data, ptr, len);

        // Decoder
        in.write(data);
        write();

        // Wait Decoder delay
        wait(sc_time(9, SC_NS));

        // Read Decoder output
        decoded_data = out.read();

        // Obliged to set response status to indicate successful completion
        trans_pending->set_response_status( tlm::TLM_OK_RESPONSE );

        cout << name() << " BEGIN_RESP SENT" << " at time " << sc_time_stamp() << endl;

        // Backward call
        delay_bw= sc_time(1, SC_NS);
        status_bw = target_socket->nb_transport_bw( *trans_pending, phase_bw, delay_bw );

        switch (status_bw) {
            case tlm::TLM_ACCEPTED:
                cout << name() << " NB_TRANSPORT_BW (STATUS TLM_ACCEPTED)" << " at time " << sc_time_stamp() << endl;
                break;
            default:
                cout << name() << " NB_TRANSPORT_BW (STATUS not expected)" << " at time " << sc_time_stamp() << endl;
                break;
        }

        // Forward call Decoder
	    tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
		delay_fw = sc_time(1, SC_NS);

		trans->set_command( tlm::TLM_WRITE_COMMAND );
        trans->set_address( SYNTHESIS_COEFF );
		trans->set_data_length( 8 );
		trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
		trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
		trans->set_dmi_allowed( false ); // Mandatory initial value
		trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

		cout << name() << " BEGIN_REQ SENT" << " at time " << sc_time_stamp() << endl;

		trans->set_data_ptr( reinterpret_cast<unsigned char*>(&decoded_data) );

		status_fw = initiator_socket->nb_transport_fw( *trans, phase_fw, delay_fw );  // Blocking transport call

        switch (status_fw) {
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

};

tlm::tlm_sync_enum HammingDec::nb_transport_bw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();

    //ID_extension* id_extension = new ID_extension;
    //trans.get_extension( id_extension );

    if (phase == tlm::BEGIN_RESP) {

        // Initiator obliged to check response status   
        if (trans.is_response_error()) {
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport");
        }

        //Delay
        wait(delay);

        //cout << name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
        cout << name() << " BEGIN_RESP RECEIVED" << " at time " << sc_time_stamp() << endl;
    }

    return tlm::TLM_ACCEPTED;
};

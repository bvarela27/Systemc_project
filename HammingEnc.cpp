#include <iostream>
#include <systemc.h>
#include <stdlib.h>
#include "HammingEnc.h"
#include "register_map.h"
#include "lpc_analizer.h"

void HammingEnc::encode() {
    wait(enc_t);
    temp[0]  = reg[0] ^reg[1] ^reg[3] ^reg[4] ^reg[6] ^reg[8] ^reg[10]^reg[11]^reg[13]^reg[15]^reg[17]^reg[19]^reg[21]^reg[23]^reg[25];
    temp[1]  = reg[0] ^reg[2] ^reg[3] ^reg[5] ^reg[6] ^reg[9] ^reg[10]^reg[12]^reg[13]^reg[16]^reg[17]^reg[20]^reg[21]^reg[24]^reg[25];
    temp[3]  = reg[1] ^reg[2] ^reg[3] ^reg[7] ^reg[8] ^reg[9] ^reg[10]^reg[14]^reg[15]^reg[16]^reg[17]^reg[22]^reg[23]^reg[24]^reg[25];
    temp[7]  = reg[4] ^reg[5] ^reg[6] ^reg[7] ^reg[8] ^reg[9] ^reg[10]^reg[18]^reg[19]^reg[20]^reg[21]^reg[22]^reg[23]^reg[24]^reg[25];
    temp[15] = reg[11]^reg[12]^reg[13]^reg[14]^reg[15]^reg[16]^reg[17]^reg[18]^reg[19]^reg[20]^reg[21]^reg[22]^reg[23]^reg[24]^reg[25];
    temp[2]  = reg[0];
    temp[4]  = reg[1];
    temp[5]  = reg[2];
    temp[6]  = reg[3];
    temp[8]  = reg[4];
    temp[9]  = reg[5];
    temp[10] = reg[6];
    temp[11] = reg[7];
    temp[12] = reg[8];
    temp[13] = reg[9];
    temp[14] = reg[10];
    temp[16] = reg[11];
    temp[17] = reg[12];
    temp[18] = reg[13];
    temp[19] = reg[14];
    temp[20] = reg[15];
    temp[21] = reg[16];
    temp[22] = reg[17];
    temp[23] = reg[18];
    temp[24] = reg[19];
    temp[25] = reg[20];
    temp[26] = reg[21];
    temp[27] = reg[22];
    temp[28] = reg[23];
    temp[29] = reg[24];
    temp[30] = reg[25];
    temp[31] = reg[26];
};

void HammingEnc::write(uint32_t in) {
   reg = (sc_bv<32>) in;
   enc_t.notify(10, SC_NS);
};

uint32_t HammingEnc::read() {
    return (sc_uint<32>)temp;
};

tlm::tlm_sync_enum HammingEnc::nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    addr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    if (phase == tlm::BEGIN_REQ) {
        // Check len

        // Queue transaction
        //trans_pending[0] = &trans;
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

void HammingEnc::thread_process() {
    tlm::tlm_phase phase_bw = tlm::BEGIN_RESP;
    tlm::tlm_phase phase_fw = tlm::BEGIN_REQ;

    tlm::tlm_sync_enum status_bw, status_fw;
    sc_time delay_bw, delay_fw;
    double data;
    uint32_t encoded_data;

    while (true) {
        wait(event_thread_process);

        // Execute Encoder with the information received
        tlm::tlm_generic_payload* trans_pending = queue_trans_pending.back();
        queue_trans_pending.pop_back();

        unsigned char*   ptr = trans_pending->get_data_ptr();
        unsigned int     len = trans_pending->get_data_length();

        memcpy(&data, ptr, len);

        // Encoder
        write(data);

        // Wait Encoder delay
        wait(sc_time(11, SC_NS));

        // Read Encoder output
        encoded_data = read();

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
        trans->set_address( DECODER_COEFF );
		trans->set_data_length( 8 );
		trans->set_streaming_width( 8 ); // = data_length to indicate no streaming
		trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
		trans->set_dmi_allowed( false ); // Mandatory initial value
		trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

		cout << name() << " BEGIN_REQ SENT" << " at time " << sc_time_stamp() << endl;

		trans->set_data_ptr( reinterpret_cast<unsigned char*>(&encoded_data) );

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

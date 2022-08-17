#include "systemc-ams.h"
#include "generic_initiator_target.h"
#include "Channel.h"

int sc_main(int argc, char* argv[]) {
    ////////////////////////////////////////////////////
    // Channel
	Channel channel0("Channel");

	////////////////////////////////////////////////////
    // generic_initiator_target
    generic_initiator_target generic_initiator_target0("generic_initiator_target");

	////////////////////////////////////////////////////
    // Router
    channel0.initiator_socket.bind( generic_initiator_target0.target_socket );
    generic_initiator_target0.initiator_socket.bind( channel0.target_socket );

  	// tracing
  	sca_util::sca_trace_file* atf = sca_util::sca_create_vcd_trace_file( "channel.vcd" );
  	sca_util::sca_trace( atf, channel0.prot_gen_out, "prot_gen_out" );
  	sca_util::sca_trace( atf, channel0.mod_out, "mod_out" );
  	sca_util::sca_trace( atf, channel0.demod0.rc.out, "rec_out" );
  	sca_util::sca_trace( atf, channel0.demod0.lp.out, "filter_out" );
 	sca_util::sca_trace( atf, channel0.demod_out, "demod_out" );

  	std::cout << "Simulation started..." << std::endl;

	sc_core::sc_start(7, sc_core::SC_US);

	channel0.prot_gen0.store_data(8624);

	sc_core::sc_start(200, sc_core::SC_US);

	channel0.prot_gen0.store_data(8);

	sc_core::sc_start(200, sc_core::SC_US);
	channel0.prot_gen0.store_data(234276);
	channel0.prot_gen0.store_data(9908612);
	channel0.prot_gen0.store_data(1997);

  	sc_core::sc_start(10, sc_core::SC_MS);

  	std::cout << "Simulation finished." << std::endl;

  	sca_util::sca_close_vcd_trace_file( atf );

  	return 0;
}

#include "lpc_synthesis.h"
#include "register_map.h"

//Decoder Function
arma::vec lpc_synthesis::lcpDecode(arma::vec A, double *GFE){

    double G = GFE[0];
    double F = GFE[1];

    arma::vec xhat(WINDOW_LENGTH_2);
    xhat.zeros();

    arma::vec src(WINDOW_LENGTH_2);
    src.zeros();

    uint16_t step = 0;
    static uint16_t offset = 0;

    if (F>0)
    {
        step = round(1/F);
        if (step < WINDOW_LENGTH_2)
        {
            for(uint16_t i = offset; i<WINDOW_LENGTH_2; i+=step)
            {
                src(i) = sqrt(step);
                offset = step + i - WINDOW_LENGTH_2+1;
                //std::cout<< i << "OFF      "<<offset<<std::endl;
            }
        }
    }

    else
    {
        offset = 0;
        for(uint16_t i = 0; i<WINDOW_LENGTH_2; i++)
        {
            src = arma::randn(WINDOW_LENGTH_2);
        }
    }
    arma::vec A_0(1.0);
    A_0(0) = -1;
    arma::vec b = {1.0};
    A=join_cols(A_0,A);
    xhat = filter(b,A,sqrt(G)*src);


    /*if (lowcut > 0){
        std::cout<<"butterworth..."<<std::endl;
        xhat = filter(b_butterworth,a_butterworth,xhat);
    }*/

    return xhat;
}

//Filter Function
arma::vec lpc_synthesis::filter(arma::vec b, arma::vec a, arma::vec X) {

    if (b.n_rows < a.n_rows)
    {
        b = arma::join_vert(b,arma::zeros<arma::vec>(a.n_rows-1));
    }
    else if(b.n_rows > a.n_rows){
        a = arma::join_vert(a,arma::zeros<arma::vec>(b.n_rows-1));
    }

    int n = a.n_rows;

    arma::vec z(n);
    z.zeros();

    arma::vec Y(X.n_rows);
    Y.zeros();

    b = b/a(0);
    a = a/a(0);

    if(a.n_rows>1)
    {
        for(uint16_t m = 0; m < Y.n_rows; m++)
        {
            Y(m) = b(0)*X(m) + z(0);
            for(uint16_t i = 1; i < n-1; i++)
            {
                z(i-1) = (b(i)*X(m))+z(i)-(a(i)*Y(m));
            }

            z(n-2)=(b(n-1)*X(m)) - (a(n-1)*Y(m));
        }
        //z(a.n_rows)
    }
    else
    {
        for(uint16_t m = 0; m < X.n_rows; m++)
        {
            Y(m) = b(0)*X(m) + z(0);

            for(uint16_t i = 1; i < n; i++)
            {
                z(i-1) = b(i)*X(m)+z(i);
            }
            z(n-2)=b(n-1)*X(m);
        }
    }


    //z = z.rows(1,n-1);

    return Y;
}

void lpc_synthesis::LPC_decoding() {
        arma::vec A(N_POLES);
        arma::vec out(WINDOW_LENGTH_2);
        out.zeros();
        double GF[2];

        for(uint16_t n = 0; n<N_POLES; n++) {
            A(n) = input_buffer[n];
        }

        GF[0] = input_buffer[N_POLES];
        GF[1] = input_buffer[N_POLES+1];

        out = lcpDecode(A,GF);

        sleep(1);
        for(uint16_t n = 0; n<WINDOW_LENGTH_2; n++) {
            LPC_output[n]=out(n);
        }
    }

void lpc_synthesis::execute(double * input) {
    std::copy(input, input + N_POLES+2, input_buffer);
    LPC_decoding();
}

double* lpc_synthesis::read_output() {
    static double output[WINDOW_LENGTH_2];
    std::copy(LPC_output,LPC_output+WINDOW_LENGTH_2,output);

    return LPC_output;
}

tlm::tlm_sync_enum lpc_synthesis::nb_transport_fw( tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay ) {
	tlm::tlm_command cmd = trans.get_command();
	sc_dt::uint64    addr = trans.get_address();
	unsigned char*   ptr = trans.get_data_ptr();
	unsigned int     len = trans.get_data_length();
	unsigned char*   byt = trans.get_byte_enable_ptr();
	unsigned int     wid = trans.get_streaming_width();
	
	wait(delay);
	
	double data;
	memcpy(&data, ptr, len);

    // Decode input_buffer index
    int input_buffer_idx;

    input_buffer_idx = (addr - SYN_POLO_0)/8;
    input_buffer[input_buffer_idx] = data;
	
	//cout << name() << " Data received: " << data << endl;

    if (phase == tlm::BEGIN_REQ) {
        cout << name() << " BEGIN_REQ RECEIVED" << " at time " << sc_time_stamp() << endl;
    }
	
	// Obliged to set response status to indicate successful completion
	trans.set_response_status( tlm::TLM_OK_RESPONSE );

    return tlm::TLM_ACCEPTED;
}

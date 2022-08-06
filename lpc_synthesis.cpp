#include "lpc_synthesis.h"
#include "register_map.h"
#include "26bit.h"

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
    ID_extension* id_extension = new ID_extension;
    trans.get_extension(id_extension);

    if (phase == tlm::BEGIN_REQ) {
        // Check len

        // Queue transaction
        trans_pending.push(&trans);
        phase_pending=phase;

        // Delay
        wait(delay);

        // Trigger event
        event_thread_process.notify();

        // Display message
        cout << name() << " BEGIN_REQ RECEIVED " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

        // Obliged to set response status to indicate successful completion
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    } else {
        cout << name() << " NB_TRANSPORT_FW unexpected phase" << " at time " << sc_time_stamp() << endl;
        trans.set_response_status( tlm::TLM_GENERIC_ERROR_RESPONSE );
    }
    return tlm::TLM_ACCEPTED;
};

void lpc_synthesis::thread_notify() {
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

void lpc_synthesis::thread_process() {
    tlm::tlm_phase phase_bw = tlm::BEGIN_RESP;
    tlm::tlm_sync_enum status_bw;
    sc_time delay_bw;
    int32_t data;
    uint32_t encoded_data;
    int count_coeff = 0;
    double coeffs[N_POLES+2];
    tlm::tlm_generic_payload * current_trans;

    while (true) {

        wait(event_thread_process);
        // Execute Encoder with the information received
        current_trans = trans_pending.front();
        trans_pending.pop();
        unsigned char*   ptr = current_trans->get_data_ptr();
        unsigned int     len = current_trans->get_data_length();
        ID_extension* id_extension = new ID_extension;
        current_trans->get_extension( id_extension );

        data = *reinterpret_cast<int32_t*>(ptr);

        // Store in buffer
        coeffs[count_coeff] = FixedToDouble(data);
        if (count_coeff == N_POLES){
            coeffs[count_coeff] = coeffs[count_coeff]/1000;
        }
        cout<<"SYNTH "<< coeffs[count_coeff]<<endl;
        // Check if all the coeffs are ready to be processed
        if (count_coeff == N_POLES+2-1) {
            // Execute synthesis
            execute(coeffs);

            // FIXME
            // Read synthesis results

            // Restart counter
            count_coeff = 0;
        } else {
            count_coeff++;
        }

        // Obliged to set response status to indicate successful completion
        current_trans->set_response_status( tlm::TLM_OK_RESPONSE );

        // Backward call
        cout << name() << " BEGIN_RESP SENT" << "    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
        delay_bw= sc_time(1, SC_NS);
        status_bw = socket->nb_transport_bw( *current_trans, phase_bw, delay_bw );

        switch (status_bw) {
            case tlm::TLM_ACCEPTED:
                cout << name() << " NB_TRANSPORT_BW (STATUS TLM_ACCEPTED)" << " at time " << sc_time_stamp() << endl;
                break;
            default:
                cout << name() << " NB_TRANSPORT_BW (STATUS not expected)" << " at time " << sc_time_stamp() << endl;
                break;
        }
        done.notify();
    }
};

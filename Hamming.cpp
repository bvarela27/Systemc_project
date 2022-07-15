#include <iostream>
#include <systemc.h>
#include <stdlib.h>
#define N_HAMMING 31
#define K_HAMMING 26
#define M_HAMMING N_HAMMING-K_HAMMING

SC_MODULE(HammingEnc)
{
    sc_event enc_t;
    sc_bv<32> reg = 0;
    sc_bv<32> temp = 0;

    SC_CTOR(HammingEnc)
    {
        SC_THREAD(encode);
    }

    void encode()
    {
        wait(enc_t);
        std::cout<<reg<<std::endl;
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
        std::cout<<temp<<std::endl;
    }

    void write(uint32_t in)
    {
       reg = (sc_bv<32>) in;
       enc_t.notify(10, SC_NS);
    }

    uint32_t read()
    {
        return (sc_uint<32>)temp;
    }

};


SC_MODULE(HammingDec)
{
    sc_event ecc_t, dec_t;
    sc_in< sc_uint<32> > in;
    sc_out< sc_uint<32> > out;
    sc_bv<32> reg, decoded_val;
    sc_bv<32> syn;
    SC_CTOR(HammingDec)
    {
        SC_THREAD(decode);
        SC_THREAD(ECC);
    }

    void decode()
    {
        while(true)
        {
            wait(dec_t);
            reg = (sc_bv<32>) in;
            syn[0] = reg[0] ^reg[2] ^reg[4] ^reg[6] ^reg[8] ^reg[10]^reg[12]^reg[14]^reg[16]^reg[18]^reg[20]^reg[22]^reg[24]^reg[26]^reg[28]^reg[30];
            syn[1] = reg[1] ^reg[2] ^reg[5] ^reg[6] ^reg[9] ^reg[10]^reg[13]^reg[14]^reg[17]^reg[18]^reg[21]^reg[22]^reg[25]^reg[26]^reg[29]^reg[30];
            syn[2] = reg[3] ^reg[4] ^reg[5] ^reg[6] ^reg[11]^reg[12]^reg[13]^reg[14]^reg[19]^reg[20]^reg[21]^reg[22]^reg[27]^reg[28]^reg[29]^reg[30];
            syn[3] = reg[7] ^reg[8] ^reg[9] ^reg[10]^reg[11]^reg[12]^reg[13]^reg[14]^reg[23]^reg[24]^reg[25]^reg[26]^reg[27]^reg[28]^reg[29]^reg[30];
            syn[4] = reg[15]^reg[16]^reg[17]^reg[18]^reg[19]^reg[20]^reg[21]^reg[22]^reg[23]^reg[24]^reg[25]^reg[26]^reg[27]^reg[28]^reg[29]^reg[30];
            ecc_t.notify(6,SC_NS);
        }
    }

    void ECC(){
        while(true)
        {
            wait(ecc_t);
            sc_uint<32> syn_b =(sc_uint<32>)syn;
            std::cout<<"dec reg "<<reg<<std::endl;
            if ( syn_b > 0 )
            {
                std::cout<<"dec syn "<<syn_b<<std::endl;
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
            std::cout<<"dec reg "<<reg<<std::endl;
        }


    }

    void write()
    {
       dec_t.notify(2, SC_NS);
    }
};

int sc_main(int argc, char **argv)
{
    sc_signal<sc_uint<32> > data;
    sc_signal<sc_uint<32> > out_data;
    HammingEnc encoder("encoder");
    HammingDec decoder("decoder");
        decoder.in(data);
        decoder.out(out_data);
    sc_start(0,SC_NS);
    uint32_t target = 37450726;
    encoder.write(target);
    sc_start(20,SC_NS);
    uint32_t val = encoder.read();
    srand(time(NULL));
    uint8_t error_bit= (rand() % 26);
    std::cout<<error_bit<<std::endl;
    data = val ^ (1<<error_bit);
    decoder.write();
    sc_start(20,SC_NS);
    std::cout<<out_data<<std::endl;

    return 0;
}

#ifndef ADC_CONVERTER_H
#define ADC_CONVERTER_H

#include <systemc-ams>
#include <math.h>

using namespace std;

template<int NBits>
SCA_TDF_MODULE(adc_converter) {
    // Events to trigger the TLM logic
    sc_event io_request;

    sca_tdf::sca_in<double> in;
    sca_tdf::sca_de::sca_out<sc_dt::sc_int<NBits>> out;

    adc_converter(sc_core::sc_module_name nm, double v_max_,
        sca_core::sca_time Tm_ = sca_core::sca_time(125, sc_core::SC_MS))
        : in("in"), out("out"), v_max(v_max_), Tm(Tm_) {
            sc_assert((2 <= NBits) && (NBits <= 64));
            sc_assert(v_max_ > 0.0);
    }

    void initialize() {
        change_attribute_en = 0;
        adc_sample_frequency = 8000;
    }

    void set_sample_frequency(double adc_sample_frequency_) {
        change_attribute_en = 1;
        adc_sample_frequency = adc_sample_frequency_;
    }

    void set_attributes() {
        set_timestep(Tm);

        // To allow the module to change the attributes
        // This is needed because the sample rate can be changed through a register
        does_attribute_changes(); 
    }

    void change_attributes() {
        if (change_attribute_en) {
            change_attribute_en = 0;
            set_timestep((1.0/adc_sample_frequency), sc_core::SC_SEC);
        }
    }

    void processing() {
        double v_in = in.read();

        if (v_in < -v_max) {
            out.write(-(pow(2, NBits-1)-1));
        } else if (v_in > v_max) {
            out.write((pow(2, NBits-1)-1));
        } else {
            sc_dt::sc_int<NBits> q_v_in = lround((v_in / v_max) * (pow(2, NBits-1)-1));
            out.write(q_v_in);
        }

        // Trigger event
        io_request.notify();
    }

    private:
        bool change_attribute_en;
        double adc_sample_frequency;
        const double v_max;
        sca_core::sca_time Tm;
};

#endif

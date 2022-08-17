export SYSTEMC_HOME=/usr/local/systemc-2.3.2
export LD_LIBRARY_PATH=$(SYSTEMC_HOME)/lib-linux64

compile_tlm_only:
	g++ -I$(SYSTEMC_HOME)/include -L$(SYSTEMC_HOME)/lib-linux64 tlm_tb.cpp lpc_analizer.cpp lpc_synthesis.cpp HammingEnc.cpp HammingDec.cpp \
	pitch.c fft.c singular_value_decomposition.c support_math.c -lsystemc -lm -o lpc.o -larmadillo

run_tlm_only:
	./lpc.o
    
    
    
    
    

    
    
    

    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    

    
    
    
    
    
    

    
    
    

    
    
    
    
    
    

    


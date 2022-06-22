export SYSTEMC_HOME=/usr/local/systemc-2.3.2
export LD_LIBRARY_PATH=$(SYSTEMC_HOME)/lib-linux64

compile:
	g++ -I$(SYSTEMC_HOME)/include -L$(SYSTEMC_HOME)/lib-linux64 lpc_analizer_tb.cpp lpc_analizer.cpp -lsystemc -lm -o lpc_analizer.o -larmadillo

run:
	./lpc_analizer.o
    
    
    
    
    

    
    
    

    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    

    
    
    
    
    
    

    
    
    

    
    
    
    
    
    

    


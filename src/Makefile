SCRIPT=hls.tcl
DEFAULT_OUTPUT_DIR=current_design

hls_build:
	vitis_hls -f $(SCRIPT) 

check:
	cat $(DEFAULT_OUTPUT_DIR)/solution/syn/report/krnl_csynth.rpt    

clean:
	rm -rf $(DEFAULT_OUTPUT_DIR)
	rm *log

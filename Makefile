# Commands to build xo and xclbin objects

CLK_FREQ=300

SRC_PATH=./src

PRE_TCL_FILE=$(SRC_PATH)/runPre.tcl
CONFIG_FILE=$(SRC_PATH)/u250-krnl.cfg
SOURCE_FILE=$(SRC_PATH)/krnl.c

XO_NAME=krnl.xo
XCLBIN_NAME=krnl.xclbin

MODE=hw
OPT_LEVEL=3 # Optimize level
RPT_LEVEL=2 # Report level

xo: ./$(SOURCE_FILE)
	v++ -c -R$(RPT_LEVEL) --kernel_frequency $(CLK_FREQ) -t $(MODE) \
		--hls.pre_tcl $(PRE_TCL_FILE) \
		--config $(CONFIG_FILE) -k krnl \
		-I$(SRC_PATH) $(SOURCE_FILE) -o $(XO_NAME)

xclbin: xo
	v++ -l -R$(RPT_LEVEL) -O $(OPT_LEVEL) --kernel_frequency $(CLK_FREQ) \
		-t $(MODE) --config $(CONFIG_FILE) $(XO_NAME) -o $(XCLBIN_NAME)

clean:
	rm -rf test *ltx *csv *summary _x xilinx* .run .Xil .ipcache *.jou

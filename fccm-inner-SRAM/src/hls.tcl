open_project "current_design" 
set_top krnl
add_files "./krnl.c"  
add_files "./krnl.h"
open_solution "solution" -flow_target vitis

set_part {xcu250-figd2104-2L-e}
create_clock -period 3.3333 -name krnl_clk
csynth_design
# export_design -format ip_catalog -flow impl
quit

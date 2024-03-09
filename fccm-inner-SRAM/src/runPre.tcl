# set_clock_uncertainty 40%
# config_interface -m_axi_latency 128

config_interface -register_io scalar_all
config_rtl -register_all_io
config_schedule -enable_dsp_full_reg=true

config_op dadd -impl fabric
config_op dsub -impl fabric

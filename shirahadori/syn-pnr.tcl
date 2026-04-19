set_device -name "GW1NR-9C" "GW1NR-LV9QN88PC6/I5"

set_option -verilog_std sysv2017
set_option -print_all_synthesis_warning 1
set_option -top_module "top"
set_option -output_base_name "project"
set_option -use_sspi_as_gpio 1

add_file "src/port.cst"
add_file "src/top.sv"

run all

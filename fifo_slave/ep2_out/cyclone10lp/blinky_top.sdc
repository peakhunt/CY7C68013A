create_clock -name {clk_50} -period 20.000 [get_ports {clk_50}]
create_clock -name {fx2_ifclk_48} -period 20.833 [get_ports {fx2_ifclk}]

derive_pll_clocks
derive_clock_uncertainty

# Isolates the 48MHz reference from the 100MHz internal grid automatically
set_clock_groups -asynchronous \
    -group [get_clocks {clk_50}] \
    -group [get_clocks {fx2_ifclk_48}] \
    -group [get_clocks {*pll*|clk[*]}]


set_false_path -from [get_ports {rst_n fx2_flaga fx2_fd[*]}] -to *
set_false_path -to [get_ports {leds[*] fx2_sloe fx2_slrd}]
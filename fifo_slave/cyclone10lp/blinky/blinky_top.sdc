# =========================================================================
# 1. Primary Clock Definition (50 MHz Entry Core on Pin 91)
# =========================================================================
create_clock -name {clk_50} -period 20.000 [get_ports {clk_50}]

# =========================================================================
# 2. Automated PLL Internal Clock Derivation
# =========================================================================
derive_pll_clocks
derive_clock_uncertainty

# =========================================================================
# 3. Clock Domain Crossing (CDC) Boundary Isolation (MBS CLASS FIXED)
# =========================================================================
# This cuts the timing paths between your 100MHz logic engine and 48MHz bus
#set_clock_groups -asynchronous \
#    -group [get_clocks {pll_inst|altpll_component|auto_generated|pll1|clk[0]}] \
#    -group [get_clocks {pll_inst|altpll_component|auto_generated|pll1|clk[1]}]

# =========================================================================
# 4. Asynchronous Path Masking & I/O Delays
# =========================================================================
set_false_path -from [get_ports {rst_n}] -to *

# Output delays are now correctly bound to the 100 MHz logic clock (clk[0]) 
# that drives your toggler registers.
set_output_delay -clock {pll_inst|altpll_component|auto_generated|pll1|clk[0]} -max 5.000 [get_ports {leds[*]}]
set_output_delay -clock {pll_inst|altpll_component|auto_generated|pll1|clk[0]} -min -1.000 [get_ports {leds[*]}]
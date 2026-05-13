# =========================================================================
# 1. Primary Clock Definition (50 MHz Input Core on Pin 91)
# =========================================================================
create_clock -name {clk_50} -period 20.000 [get_ports {clk_50}]

# =========================================================================
# 2. Automated PLL Internal Clock Derivation
# =========================================================================
derive_pll_clocks
derive_clock_uncertainty

# =========================================================================
# 3. Clock Domain Crossing (CDC) Boundary Isolation
# =========================================================================
# Cuts timing paths crossing between your 50MHz crystal clock and 100MHz logic core
set_clock_groups -asynchronous \
    -group [get_clocks {clk_50}] \
    -group [get_clocks {pll_inst|altpll_component|auto_generated|pll1|clk[0]}]

# =========================================================================
# 4. Asynchronous Path Masking & LED Pins Isolation
# =========================================================================
# Ignore timing requirements on the asynchronous reset input pin
set_false_path -from [get_ports {rst_n}] -to *

# Tell Quartus that LED toggling speed does not need nanosecond-level optimization
set_false_path -to [get_ports {leds[*]}]
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity fx2_slave_fifo_if is
port (
  -- Physical Pins connected directly to the CY7C68013A (FX2LP)
  fx2_ifclk   : in  std_logic;                     -- 48 MHz External Input Clock
  fx2_flaga   : in  std_logic;                     -- Active High (1 = Data Ready, 0 = Empty)
  fx2_fd      : in  std_logic_vector(7 downto 0);  -- 8-bit Parallel Data Bus
  fx2_sloe    : out std_logic;                     -- Slave Output Enable (Active Low)
  fx2_slrd    : out std_logic;                     -- Slave Read Strobe (Active Low)

  -- Clean Logic Core Outputs to feed into the internal Dual-Clock FIFO
  pipe_wdata  : out std_logic_vector(7 downto 0);  -- Valid byte stream payload
  pipe_wrreq  : out std_logic                      -- Write Request strobe to internal FIFO
);
end entity fx2_slave_fifo_if;

architecture rtl of fx2_slave_fifo_if is
begin

  -- 1. Persistent Output Enable Activation
  -- For a dedicated read-only data sink, we pull Slave Output Enable (SLOE) 
  -- low permanently to keep the FX2LP internal pin output drivers continuously active.
  fx2_sloe <= '0';

  -- 2. Immediate Asynchronous Bypass Read Logic
  -- Since the FX2LP is in AUTOOUT mode, the data on the bus pins remains valid
  -- for the duration of the ready flag window. We assert the Active-Low Slave 
  -- Read (SLRD) strobe immediately whenever the active-high FLAGA goes high.
  fx2_slrd <= not fx2_flaga;

  -- 3. Direct Clock Domain Signal Mapping
  -- We route the physical parallel data bus and the ready strobe straight into 
  -- your internal FIFO write ports to safely handle the 48 MHz clock window.
  pipe_wdata <= fx2_fd;
  pipe_wrreq <= fx2_flaga;

end architecture rtl;

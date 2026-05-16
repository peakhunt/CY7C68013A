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
--  signal flaga_internal: std_logic := '0';
begin

  --
  -- signals are already synchronized to fx2_ifclk
  --
  fx2_sloe <= '0';
  fx2_slrd <= not fx2_flaga;
  pipe_wdata <= fx2_fd;
  pipe_wrreq <= fx2_flaga;
 
--  process(fx2_ifclk)
--  begin
--    if rising_edge(fx2_ifclk) then
--      flaga_internal <= fx2_flaga;
--    end if;
--  end process;
--
--  fx2_slrd <= not flaga_internal;
--  fx2_sloe <= not flaga_internal;
--  pipe_wdata <= fx2_fd;
--  pipe_wrreq <= flaga_internal;

end architecture rtl;

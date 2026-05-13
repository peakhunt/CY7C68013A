library ieee;
use ieee.std_logic_1164.all;

entity reset_sync is
port (
  clk        : in  std_logic;
  ext_rst_n  : in  std_logic;
  sync_rst_n : out std_logic
 );
end entity reset_sync;

architecture rtl of reset_sync is
  signal r_sync_reg : std_logic_vector(1 downto 0) := "00";
begin

  process(clk, ext_rst_n)
  begin
    if ext_rst_n = '0' then
      -- Assert instantly when the physical button/signal drops to 0
      r_sync_reg <= "00";
    elsif rising_edge(clk) then
      -- Shift in a '1' cleanly on the clock edge
      r_sync_reg(0) <= '1';
      r_sync_reg(1) <= r_sync_reg(0);
    end if;
  end process;

  sync_rst_n <= r_sync_reg(1);

end architecture rtl;

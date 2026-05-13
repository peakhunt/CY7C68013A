library ieee;
use ieee.std_logic_1164.all;

entity toggler is
port (
  clk       : in  std_logic;
  rst_n     : in  std_logic;
  strobe_in : in  std_logic; -- Connects to counter overflow output
  state_out : out std_logic  -- Connects directly to physical LED pin
);
end entity toggler;

architecture rtl of toggler is
  signal r_state : std_logic := '0';
begin

  process(clk)
  begin
    if rising_edge(clk) then
      if rst_n = '0' then
        r_state <= '0';
      elsif strobe_in = '1' then
        r_state <= not r_state; -- Flips state only when counter flashes overflow
      end if;
    end if;
  end process;

  state_out <= r_state;

end architecture rtl;

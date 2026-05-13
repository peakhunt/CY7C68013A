library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity counter is
generic (
    WIDTH : integer := 16  -- Default 16-bit, can be adjusted
);
port (
  clk      : in  std_logic;
  rst_n    : in  std_logic;
  max_val  : in  unsigned(WIDTH-1 downto 0);
  count    : out unsigned(WIDTH-1 downto 0);
  overflow : out std_logic
);
end entity counter;

architecture rtl of counter is
  signal r_count : unsigned(WIDTH-1 downto 0) := (others => '0');
begin

  process(clk)
  begin
    if rising_edge(clk) then
      if rst_n = '0' then
        r_count  <= (others => '0');
        overflow <= '0';
      else
        if r_count >= max_val then
          r_count  <= (others => '0');
          overflow <= '1';
        else
          r_count  <= r_count + 1;
          overflow <= '0';
        end if;
      end if;
    end if;
  end process;

  count <= r_count;

end architecture rtl;

--library ieee;
--use ieee.std_logic_1164.all;
--use ieee.numeric_std.all;
--
--entity counter is
--generic (
--    WIDTH : integer := 16  -- Default 16-bit
--);
--port (
--  clk      : in  std_logic;
--  rst_n    : in  std_logic;
--  max_val  : in  unsigned(WIDTH-1 downto 0);
--  count    : out unsigned(WIDTH-1 downto 0);
--  overflow : out std_logic
--);
--end entity counter;
--
--architecture rtl of counter is
--  signal r_count       : unsigned(WIDTH-1 downto 0) := (others => '0');
--  -- Pipelined flag to store the comparison result
--  signal hit_max_next : std_logic := '0'; 
--begin
--
--  -- Look-ahead process: Evaluates the comparison separately to break the logic chain
--  process(clk)
--  begin
--    if rising_edge(clk) then
--      if rst_n = '0' then
--        hit_max_next <= '0';
--      else
--        -- Check if the NEXT value will hit or exceed max_val
--        if (r_count + 1) >= max_val then
--          hit_max_next <= '1';
--        else
--          hit_max_next <= '0';
--        end if;
--      end if;
--    end if;
--  end process;
--
--  -- Main Counter Process: Uses the pre-calculated flag for instantaneous switching
--  process(clk)
--  begin
--    if rising_edge(clk) then
--      if rst_n = '0' then
--        r_count  <= (others => '0');
--        overflow <= '0';
--      else
--        if hit_max_next = '1' then
--          r_count  <= (others => '0');
--          overflow <= '1';
--        else
--          r_count  <= r_count + 1;
--          overflow <= '0';
--        end if;
--      end if;
--    end if;
--  end process;
--
--  count <= r_count;
--
--end architecture rtl;

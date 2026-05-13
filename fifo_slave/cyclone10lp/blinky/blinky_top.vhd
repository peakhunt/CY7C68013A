library ieee;
use ieee.std_logic_1164.ALL;
use ieee.numeric_std.ALL;

entity blinky_top is
port
(
  clk_50:     in std_logic;
  rst_n:      in std_logic;
  leds:       out std_logic_vector(3 downto 0)
);
end blinky_top;

architecture arch of blinky_top is
  -- components declaration
  component sys_pll is
  port (
    inclk0 : in  std_logic := '0';
    c0     : out std_logic;        -- 50 MHz Internal Logic Clock
    locked : out std_logic         -- High when frequencies are completely stable
  );
  end component sys_pll;

  component reset_sync is
  port (
    clk        : in  std_logic;
    ext_rst_n  : in  std_logic;
    sync_rst_n : out std_logic
  );
  end component reset_sync;

  component counter is
  generic (
    WIDTH : integer := 26  -- Large width to make blinking visible to human eyes
  );
  port (
    clk      : in  std_logic;
    rst_n    : in  std_logic;
    max_val  : in  unsigned(WIDTH-1 downto 0);
    count    : out unsigned(WIDTH-1 downto 0);
    overflow : out std_logic
  );
  end component counter;

  component toggler is
  port (
    clk        : in  std_logic;
    rst_n      : in  std_logic;
    strobe_in  : in  std_logic;
    state_out  : out std_logic
  );
  end component toggler;

  -- internal pure signals
  signal w_sync_rst_n : std_logic;
  signal w_overflow1  : std_logic;
  signal w_overflow2  : std_logic;
  signal w_overflow3  : std_logic;
  signal w_overflow4  : std_logic;

  -- PLL Generated Internal Clock Rails
  signal clk_internal_100 : std_logic;
  signal w_pll_locked     : std_logic;

  -- Reset Interconnect
  signal w_combined_rst_n : std_logic;

  constant T_100 : unsigned(25 downto 0) := to_unsigned(10000000, 26);
  constant T_200 : unsigned(25 downto 0) := to_unsigned(20000000, 26);
  constant T_300 : unsigned(25 downto 0) := to_unsigned(30000000, 26);
  constant T_400 : unsigned(25 downto 0) := to_unsigned(40000000, 26);
begin

  pll_inst : sys_pll
  port map (
    inclk0 => clk_50,
    c0     => clk_internal_100,
    locked => w_pll_locked
  );

  -- We combine the physical hardware rst_n button with the PLL locked flag.
  -- If the PLL drops out of phase lock, the entire logic matrix instantly 
  -- drops into reset state to protect the internal data bus registries.
  w_combined_rst_n <= rst_n and w_pll_locked;

  reset_synchronizer : reset_sync
  port map (
    clk        => clk_internal_100,
    ext_rst_n  => w_combined_rst_n,
    sync_rst_n => w_sync_rst_n
  );

  led_counter1 : counter
  generic map (
    WIDTH => 26
  )
  port map (
    clk      => clk_internal_100,
    rst_n    => w_sync_rst_n,
    max_val  => T_100,
    count    => open,
    overflow => w_overflow1
  );

  led_counter2 : counter
  generic map (
    WIDTH => 26
  )
  port map (
    clk      => clk_internal_100,
    rst_n    => w_sync_rst_n,
    max_val  => T_200,
    count    => open,
    overflow => w_overflow2
  );

  led_counter3 : counter
  generic map (
    WIDTH => 26
  )
  port map (
    clk      => clk_internal_100,
    rst_n    => w_sync_rst_n,
    max_val  => T_300,
    count    => open,
    overflow => w_overflow3
  );

  led_counter4 : counter
  generic map (
    WIDTH => 26
  )
  port map (
    clk      => clk_internal_100,
    rst_n    => w_sync_rst_n,
    max_val  => T_400,
    count    => open,
    overflow => w_overflow4
  );

  led1_toggler: toggler
  port map (
    clk       => clk_internal_100,
    rst_n     => w_sync_rst_n,
    strobe_in => w_overflow1,
    state_out => leds(0)
  );

  led2_toggler: toggler
  port map (
    clk       => clk_internal_100,
    rst_n     => w_sync_rst_n,
    strobe_in => w_overflow2,
    state_out => leds(1)
  );

  led3_toggler: toggler
  port map (
    clk       => clk_internal_100,
    rst_n     => w_sync_rst_n,
    strobe_in => w_overflow3,
    state_out => leds(2)
  );

  led4_toggler: toggler
  port map (
    clk       => clk_internal_100,
    rst_n     => w_sync_rst_n,
    strobe_in => w_overflow4,
    state_out => leds(3)
  );
end arch;

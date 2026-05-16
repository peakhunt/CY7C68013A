library ieee;
use ieee.std_logic_1164.ALL;
use ieee.numeric_std.ALL;

entity blinky_top is
port
(
  clk_50:     in std_logic;
  rst_n:      in std_logic;
  leds:       out std_logic_vector(3 downto 0);

  -- FX2LP interface
  fx2_ifclk:  in  std_logic;                        -- 48Mhz clock input
  fx2_flaga:  in  std_logic;                        -- High = Data Ready, Low = Empty
  fx2_sloe:   out std_logic;                        -- Output Enable Strobe
  fx2_slrd:   out std_logic;                        -- Read Increment Strobe
  fx2_fd:     in  std_logic_vector(7 downto 0)      -- 8bit Parallel Data
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

  component fx2_slave_fifo_if is
  port (
    fx2_ifclk   : in  std_logic;
    fx2_flaga   : in  std_logic;
    fx2_fd      : in  std_logic_vector(7 downto 0);
    fx2_sloe    : out std_logic;
    fx2_slrd    : out std_logic;
    pipe_wdata  : out std_logic_vector(7 downto 0);
    pipe_wrreq  : out std_logic
  );
  end component fx2_slave_fifo_if;

  component mbs_dcfifo_bridge IS
  port (
    aclr    : IN  STD_LOGIC := '0';
    data    : IN  STD_LOGIC_VECTOR (7 DOWNTO 0);
    rdclk   : IN  STD_LOGIC ;
    rdreq   : IN  STD_LOGIC ;
    wrclk   : IN  STD_LOGIC ;
    wrreq   : IN  STD_LOGIC ;
    q       : OUT STD_LOGIC_VECTOR (7 DOWNTO 0);
    rdempty : OUT STD_LOGIC 
  );
  end component mbs_dcfifo_bridge;

  -- PLL Generated Internal Clock Rails
  signal clk_sys_100      : std_logic;
  signal w_pll_locked     : std_logic;
  signal w_master_clear   : std_logic;

  -- Internal copper interconnect wires joining your interface to the FIFO
  signal w_bridge_wdata  : std_logic_vector(7 downto 0);
  signal w_bridge_wrreq  : std_logic;

  -- 100 MHz Capture Core domain registers
  signal w_fifo_rdata    : std_logic_vector(7 downto 0);
  signal w_fifo_rdreq    : std_logic;
  signal w_fifo_empty    : std_logic;
  signal r_data_register : std_logic_vector(7 downto 0) := (others => '0');
begin

  pll_inst : sys_pll
  port map (
    inclk0 => clk_50,
    c0     => clk_sys_100,
    locked => w_pll_locked
  );

  -- We combine the physical hardware rst_n button with the PLL locked flag.
  -- If the PLL drops out of phase lock, the entire logic matrix instantly 
  -- drops into reset state to protect the internal data bus registries.
  w_master_clear <= not (rst_n and w_pll_locked);

  -- =========================================================================
  -- LINK 1 (48 MHz Domain): The Wire-Bridge Interface Extractor
  -- =========================================================================
  u_fx2_extractor : fx2_slave_fifo_if
  port map (
    fx2_ifclk   => fx2_ifclk,
    fx2_flaga   => fx2_flaga,
    fx2_fd      => fx2_fd,
    fx2_sloe    => fx2_sloe,
    fx2_slrd    => fx2_slrd,
    pipe_wdata  => w_bridge_wdata, 
    pipe_wrreq  => w_bridge_wrreq  
  );

  -- =========================================================================
  -- LINK 2 (Silicon Threshold): Asynchronous Cross-Domain Dual-Clock FIFO
  -- =========================================================================
  u_silicon_bridge : mbs_dcfifo_bridge
  port map (
    aclr    => w_master_clear,
    wrclk   => fx2_ifclk,          -- 48 MHz launch domain clock (wrclk pin)
    wrreq   => w_bridge_wrreq,     -- High when FLAGA hits (wrreq pin)
    data    => w_bridge_wdata,     -- 8-bit raw bus data payload (data pin)
    rdclk   => clk_sys_100,        -- 100 MHz capture domain clock (rdclk pin)
    rdreq   => w_fifo_rdreq,       -- Controlled by 100 MHz process below (rdreq pin)
    q       => w_fifo_rdata,       -- Safe, synchronized data byte out (q pin)
    rdempty => w_fifo_empty        -- '0' means valid USB payload is present (rdempty pin)
  );

  -- =========================================================================
  -- LINK 3 (100 MHz Domain): Pure Synchronous Internal Core Data Capture
  -- =========================================================================
  process(clk_sys_100)
  begin
    if rising_edge(clk_sys_100) then
      if w_master_clear = '1' then
        w_fifo_rdreq    <= '0';
        r_data_register <= (others => '0');
      else
        w_fifo_rdreq <= '0';

        -- 1. Check empty and request the read
        if w_fifo_empty = '0' then
          w_fifo_rdreq <= '1';
        end if;

        -- 2. This creates the mandatory 1-cycle delay to capture the new data safely
        if w_fifo_rdreq = '1' then
          r_data_register <= w_fifo_rdata;
        end if;
      end if;
    end if;
  end process;

  leds <= r_data_register(7 downto 4);
end arch;

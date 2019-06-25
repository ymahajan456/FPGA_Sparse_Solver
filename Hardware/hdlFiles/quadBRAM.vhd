library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.all;

entity quadPortBRAM is
    generic(
        ADDR_WIDTH : integer := 10;
        DATA_WIDTH : integer := 32
    );
    port
    (
        CLK_1X          : in std_logic;
        CLK_2X          : in std_logic;
        RST             : in std_logic;
        BRAM_PORTA_ADDR : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        BRAM_PORTA_DIN  : in  std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTA_DOUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTA_EN   : in  std_logic;
        BRAM_PORTA_WE   : in  std_logic;
        BRAM_PORTB_ADDR : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        BRAM_PORTB_DIN  : in  std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTB_DOUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTB_EN   : in  std_logic;
        BRAM_PORTB_WE   : in  std_logic;
        BRAM_PORTC_ADDR : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        BRAM_PORTC_DIN  : in  std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTC_DOUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTC_EN   : in  std_logic;
        BRAM_PORTC_WE   : in  std_logic;
        BRAM_PORTD_ADDR : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        BRAM_PORTD_DIN  : in  std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTD_DOUT : out std_logic_vector(DATA_WIDTH-1 downto 0);
        BRAM_PORTD_EN   : in  std_logic;
        BRAM_PORTD_WE   : in  std_logic;

        BLOCK_PORTA_ADDR: out std_logic_vector(ADDR_WIDTH-1 downto 0);
        BLOCK_PORTA_DIN : out std_logic_vector(DATA_WIDTH-1 downto 0);
        BLOCK_PORTA_DOUT: in  std_logic_vector(DATA_WIDTH-1 downto 0);
        BLOCK_PORTA_EN  : out std_logic;
        BLOCK_PORTA_WE  : out std_logic;

        BLOCK_PORTB_ADDR: out std_logic_vector(ADDR_WIDTH-1 downto 0);
        BLOCK_PORTB_DIN : out std_logic_vector(DATA_WIDTH-1 downto 0);
        BLOCK_PORTB_DOUT: in  std_logic_vector(DATA_WIDTH-1 downto 0);
        BLOCK_PORTB_EN  : out std_logic;
        BLOCK_PORTB_WE  : out std_logic
    );
end entity;

-- architecture behav of quadPortBRAM is

--     -- signal reg_portA_addr_in,   reg_portA_addr_out : std_logic_vector(ADDR_WIDTH-1 downto 0);
--     -- signal reg_portA_din_in,    reg_portA_din_out : std_logic_vector(DATA_WIDTH-1 downto 0);
--     signal reg_portA : std_logic_vector(DATA_WIDTH-1 downto 0);
--     -- signal reg_portA_en_in,     reg_portA_en_out : std_logic;
--     -- signal reg_portA_we_in,     reg_portA_we_out : std_logic;
--     signal reg_portB_addr_out : std_logic_vector(ADDR_WIDTH-1 downto 0);
--     signal reg_portB_din_out : std_logic_vector(DATA_WIDTH-1 downto 0);
--     -- signal reg_portB_dout_out : std_logic_vector(DATA_WIDTH-1 downto 0);
--     signal reg_portB_en_out : std_logic;
--     signal reg_portB_we_out : std_logic;
--     -- signal reg_portC_addr_in,   reg_portC_addr_out : std_logic_vector(ADDR_WIDTH-1 downto 0);
--     -- signal reg_portC_din_in,    reg_portC_din_out : std_logic_vector(DATA_WIDTH-1 downto 0);
--     signal reg_portC: std_logic_vector(DATA_WIDTH-1 downto 0);
--     -- signal reg_portC_en_in,     reg_portC_en_out : std_logic;
--     -- signal reg_portC_we_in,     reg_portC_we_out : std_logic;
--     signal reg_portD_addr_out : std_logic_vector(ADDR_WIDTH-1 downto 0);
--     signal reg_portD_din_out : std_logic_vector(DATA_WIDTH-1 downto 0);
--     -- signal reg_portD_dout_out : std_logic_vector(DATA_WIDTH-1 downto 0);
--     signal reg_portD_en_out : std_logic;
--     signal reg_portD_we_out : std_logic;

--     -- signal block_porta_addr, block_portb_addr : std_logic_vector(ADDR_WIDTH-1 downto 0);
--     -- signal block_porta_din, block_portb_din : std_logic_vector(DATA_WIDTH-1 downto 0);
--     -- signal block_porta_dout, block_portb_dout : std_logic_vector(DATA_WIDTH-1 downto 0);
--     -- signal block_porta_en, block_portb_en : std_logic;
--     -- signal block_porta_we, block_portb_we : std_logic;

--     signal mux_sel : std_logic;
--     signal reg_ena : std_logic;
--     signal portACOutReg0ENA : std_logic;
    
--     signal portBInReg_in, portBInReg_out : std_logic_vector(DATA_WIDTH + ADDR_WIDTH + 1 downto 0);
--     signal portDInReg_in, portDInReg_out : std_logic_vector(DATA_WIDTH + ADDR_WIDTH + 1 downto 0);

-- begin

--     mux_sel <= CLK_1X xor CLK_2X;
--     reg_ena <= not RST;
--     portACOutReg0ENA <= mux_sel and (reg_ena);

--     BLOCK_PORTA_ADDR <= BRAM_PORTA_ADDR when (mux_sel = '0') else reg_portB_addr_out;
--     BLOCK_PORTA_DIN  <= BRAM_PORTA_DIN  when (mux_sel = '0') else reg_portB_din_out;
--     BLOCK_PORTA_EN   <= BRAM_PORTA_EN   when (mux_sel = '0') else reg_portB_en_out;
--     BLOCK_PORTA_WE   <= BRAM_PORTA_WE   when (mux_sel = '0') else reg_portB_we_out;

--     BLOCK_PORTB_ADDR <= BRAM_PORTC_ADDR when (mux_sel = '0') else reg_portD_addr_out;
--     BLOCK_PORTB_DIN  <= BRAM_PORTC_DIN  when (mux_sel = '0') else reg_portD_din_out;
--     BLOCK_PORTB_EN   <= BRAM_PORTC_EN   when (mux_sel = '0') else reg_portD_en_out;
--     BLOCK_PORTB_WE   <= BRAM_PORTC_WE   when (mux_sel = '0') else reg_portD_we_out;

--     -- BRAM : entity design_BRAM_A_wrapper
--     -- port map (
--     --     BRAM_PORTA_addr => block_porta_addr,
--     --     BRAM_PORTA_clk  => CLK_2X,
--     --     BRAM_PORTA_din  => block_porta_din,
--     --     BRAM_PORTA_dout => block_porta_dout,
--     --     BRAM_PORTA_en   => block_porta_en,
--     --     BRAM_PORTA_we(0)   => block_porta_we,
--     --     BRAM_PORTB_addr => block_portb_addr,
--     --     BRAM_PORTB_clk  => CLK_2X,
--     --     BRAM_PORTB_din  => block_portb_din,
--     --     BRAM_PORTB_dout => block_portb_dout,
--     --     BRAM_PORTB_en   => block_portb_en,
--     --     BRAM_PORTB_we(0)   => block_portb_we
--     -- );

--     portBInReg_in       <= BRAM_PORTB_ADDR & BRAM_PORTB_DIN & BRAM_PORTB_EN & BRAM_PORTB_WE;
--     reg_portB_addr_out  <= portBInReg_out(portBInReg_out'length-1 downto 2 + DATA_WIDTH);
--     reg_portB_din_out   <= portBInReg_out(1 + DATA_WIDTH downto 2);
--     reg_portB_en_out    <= portBInReg_out(1);
--     reg_portB_we_out    <= portBInReg_out(0);

--     portBInReg : entity myReg
--     generic map(ADDR_WIDTH + DATA_WIDTH + 2)
--     port map(
--         CLK => CLK_1X,
--         ARST => RST,
--         ENA => reg_ena,
--         DIN => portBInReg_in,
--         DOUT => portBInReg_out
--     );

--     portBOutReg : entity myReg
--     generic map(DATA_WIDTH)
--     port map(
--         CLK => CLK_1X,
--         ARST => RST,
--         ENA => reg_ena,
--         DIN => BLOCK_PORTA_DOUT,
--         DOUT => BRAM_PORTB_DOUT
--     );

--     portAOutReg0 : entity myReg
--     generic map(DATA_WIDTH)
--     port map(
--         CLK => CLK_2X,
--         ARST => RST,
--         ENA => portACOutReg0ENA,
--         DIN => BLOCK_PORTA_DOUT,
--         DOUT => reg_portA
--     );

--     portAOutReg1 : entity myReg
--     generic map(DATA_WIDTH)
--     port map(
--         CLK => CLK_1X,
--         ARST => RST,
--         ENA => reg_ena,
--         DIN => reg_portA,
--         DOUT => BRAM_PORTA_DOUT
--     );

--     portDInReg_in <= BRAM_PORTD_ADDR & BRAM_PORTD_DIN & BRAM_PORTD_EN & BRAM_PORTD_WE;
--     reg_portD_addr_out  <= portDInReg_out(portDInReg_out'length-1 downto 2 + DATA_WIDTH);
--     reg_portD_din_out   <= portDInReg_out(1 + DATA_WIDTH downto 2);
--     reg_portD_en_out    <= portDInReg_out(1);
--     reg_portD_we_out    <= portDInReg_out(0);
    
--     portDInReg : entity myReg
--     generic map(ADDR_WIDTH + DATA_WIDTH + 2)
--     port map(
--         CLK => CLK_1X,
--         ARST => RST,
--         ENA => reg_ena,
--         DIN => portDInReg_in,
--         DOUT => portDInReg_out 
--     );

--     portDOutReg : entity myReg
--     generic map(DATA_WIDTH)
--     port map(
--         CLK => CLK_1X,
--         ARST => RST,
--         ENA => reg_ena,
--         DIN => BLOCK_PORTB_DOUT,
--         DOUT => BRAM_PORTD_DOUT
--     );

--     portCOutReg0 : entity myReg
--     generic map(DATA_WIDTH)
--     port map(
--         CLK => CLK_2X,
--         ARST => RST,
--         ENA => portACOutReg0ENA,
--         DIN => BLOCK_PORTB_DOUT,
--         DOUT => reg_portC
--     );

--     portCOutReg1 : entity myReg
--     generic map(DATA_WIDTH)
--     port map(
--         CLK => CLK_1X,
--         ARST => RST,
--         ENA => reg_ena,
--         DIN => reg_portC,
--         DOUT => BRAM_PORTC_DOUT
--     );

-- end architecture;


architecture theory of quadPortBRAM is

signal reg_ena : std_logic;

begin

reg_ena <= not RST;

BLOCK_PORTA_ADDR <= BRAM_PORTA_ADDR;
BLOCK_PORTA_DIN  <= BRAM_PORTA_DIN;
BLOCK_PORTA_EN   <= BRAM_PORTA_EN;
BLOCK_PORTA_WE   <= BRAM_PORTA_WE;

BLOCK_PORTB_ADDR <= BRAM_PORTC_ADDR;
BLOCK_PORTB_DIN  <= BRAM_PORTC_DIN;
BLOCK_PORTB_EN   <= BRAM_PORTC_EN;
BLOCK_PORTB_WE   <= BRAM_PORTC_WE;

portDOutReg : entity myReg
generic map(DATA_WIDTH)
port map(
    CLK => CLK_1X,
    ARST => RST,
    ENA => reg_ena,
    DIN => BLOCK_PORTB_DOUT,
    DOUT => BRAM_PORTD_DOUT
);

portBOutReg : entity myReg
generic map(DATA_WIDTH)
port map(
    CLK => CLK_1X,
    ARST => RST,
    ENA => reg_ena,
    DIN => BLOCK_PORTA_DOUT,
    DOUT => BRAM_PORTB_DOUT
);

portAOutReg : entity myReg
generic map(DATA_WIDTH)
port map(
    CLK => CLK_1X,
    ARST => RST,
    ENA => reg_ena,
    DIN => BLOCK_PORTB_DOUT,
    DOUT => BRAM_PORTC_DOUT
);

portCOutReg : entity myReg
generic map(DATA_WIDTH)
port map(
    CLK => CLK_1X,
    ARST => RST,
    ENA => reg_ena,
    DIN => BLOCK_PORTA_DOUT,
    DOUT => BRAM_PORTA_DOUT
);

end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.all;
use work.types.all;

entity LUDH_Tester is
    generic(
        ADDR_WIDTH : integer := 10;
        CTRL_WIDTH : integer := 340;
        TEST_LENGTH : integer := 20
    );
    port(
        CLK_IN : in std_logic;
        RST_IN : in std_logic;
        CTRL_SIGNAL : out std_logic_vector(CTRL_WIDTH-1 downto 0);
        CTRL_VALID : out std_logic;
        COMPLETED : out std_logic;
        CLK : in std_logic;
        RST : in std_logic
    );
end entity;

architecture yoStyle of LUDH_Tester is

    signal ctrl_addr, ctrl_addr_in, ctrl_last : std_logic_vector(ADDR_WIDTH-1 downto 0);
    signal f_locked, run_test, test_completed : std_logic;
begin

    ctrl_last <= std_logic_vector(to_unsigned(TEST_LENGTH, ADDR_WIDTH));
    f_locked <= not RST;

    test_completed <= '1' when(ctrl_addr = ctrl_last) else '0';
    run_test <= f_locked and not(test_completed);

    COMPLETED <= test_completed;

    ctrlAddrReg : entity myReg
    generic map(ADDR_WIDTH)
    port map(
        CLK => CLK,
        ARST => RST,
        ENA => run_test,
        DIN => ctrl_addr_in,
        DOUT => ctrl_addr
    );

    ctrl_addr_in <= std_logic_vector(to_unsigned(to_integer(unsigned(ctrl_addr)) + 1, ADDR_WIDTH));

    -- f_locked_delay : entity myReg
    -- generic map(1)
    -- port map(
    --     CLK => CLK,
    --     ARST => RST,
    --     ENA => '1',
    --     DIN(0) => f_locked,
    --     DOUT(0) => CTRL_VALID
    -- );

    CTRL_VALID <= f_locked;

    ctrlStorage : entity design_CTRL_wrapper
    port map (
    BRAM_PORTA_addr => ctrl_addr(8 downto 0) ,
    BRAM_PORTA_clk  => CLK ,
    BRAM_PORTA_din  => (others => '0') ,
    BRAM_PORTA_dout => CTRL_SIGNAL ,
    BRAM_PORTA_en   => '1' ,
    BRAM_PORTA_we   => "0"
    );

--    LUDH : entity LUDHardware
--    generic map(10, 340)
--    port map(
--        CLK_IN => CLK_IN,
--        RST_IN => RST_IN,
--        CTRL_Signal => CTRL_SIGNAL,
--        CTRL_VALID => CTRL_VALID,
--        CLK_OUT => CLK,
--        RST_OUT => RST
--    );


end architecture;
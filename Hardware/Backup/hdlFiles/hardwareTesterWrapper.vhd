
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.all;
use work.types.all;

entity LUDH_TEST_WRAPPER is
    generic(
        ADDR_WIDTH : integer := 10;
        CTRL_WIDTH : integer := 340;
        TEST_LENGTH : integer := 20
    );
    port(
        CLK_IN : in std_logic;
        RST_IN : in std_logic;
        CLK_LOCKED : out std_logic;
        COMPLETED : out std_logic
    );
end entity;

architecture justConnect of LUDH_TEST_WRAPPER is

    signal clk, rst, ctrl_valid , completed_internal : std_logic;
    signal ctrl_signal : std_logic_vector(CTRL_WIDTH-1 downto 0);

begin
    CLK_LOCKED <= clk;
    COMPLETED <= completed_internal;

    tester : entity LUDH_Tester
    generic map(ADDR_WIDTH, CTRL_WIDTH, TEST_LENGTH)
    port map(
        CLK_IN => CLK_IN,
        RST_IN => RST_IN,
        CTRL_Signal => ctrl_signal,
        CTRL_VALID => ctrl_valid,
        COMPLETED => completed_internal,
        CLK => clk,
        RST => RST
    );

    LUDH : entity LUDHardware
    generic map(ADDR_WIDTH, CTRL_WIDTH)
    port map(
        CLK_IN => CLK_IN,
        RST_IN => RST_IN,
        CTRL_Signal => ctrl_signal,
        CTRL_VALID => ctrl_valid,
        COMPLETED => completed_internal,
        CLK_OUT => clk,
        RST_OUT => rst
    );

end architecture;
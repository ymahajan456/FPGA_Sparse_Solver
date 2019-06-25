
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.all;
use work.types.all;

entity simTester is
end entity;

architecture connect of simTester is

    signal clk_in, rst_in, clk_locked, completed : std_logic;

begin

    design : entity LUDH_TEST_WRAPPER
    port map(
        CLK_IN => clk_in,
        RST_IN => rst_in,
        CLK_LOCKED => clk_locked,
        COMPLETED => completed
    );

    sinClkRst : entity design_simClkRst_wrapper
    port map(
        clk => clk_in,
        sync_rst => rst_in
    );

end architecture;
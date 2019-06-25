library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity myReg is
    generic( DATA_WIDTH : integer := 24 );
    port(
        CLK     : in std_logic;
        ARST    : in  std_logic;
        ENA     : in std_logic;
        DIN     : in std_logic_vector(DATA_WIDTH-1 downto 0);
        DOUT    : out std_logic_vector(DATA_WIDTH-1 downto 0)
    );
end entity myReg;

architecture behav of myReg is
begin
process( CLK, ENA, ARST)
    begin
    if(ARST = '1') then
        DOUT <= (others => '0');
    else
        if(rising_edge(CLK) and (ENA = '1')) then
            DOUT <= DIN;
        end if;
    end if;
end process;
end architecture behav;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity myRegS is
    generic( DATA_WIDTH : integer := 24 );
    port(
        CLK     : in std_logic;
        SRST    : in std_logic;
        ENA     : in std_logic;
        DIN     : in std_logic_vector(DATA_WIDTH-1 downto 0);
        DOUT    : out std_logic_vector(DATA_WIDTH-1 downto 0)
    );
end entity myRegS;

architecture behav of myRegS is
begin
process( CLK, ENA, SRST)
    begin
    if(rising_edge(CLK) and (ENA = '1')) then
        if(SRST = '1') then
            DOUT <= (others => '0');
        else
            DOUT <= DIN;
        end if;
    end if;
end process;
end architecture behav;

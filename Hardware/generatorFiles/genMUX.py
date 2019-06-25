import math

def genBigMUX(inWidth, muxName, dataWidth = 32, default = "'X'"):
    outFile = open("../autoHdlFiles/MUX_" + muxName + '.vhd', 'w')

    selWidth = math.ceil(math.log2(inWidth))

    outFile.write("""
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.types.all;

entity MUX_{0} is
    port(
        SEL  : in  std_logic_vector({1}-1 downto 0);
        DIN  : in  dataArray({2} downto 0);
        DOUT : out std_logic_vector({3} downto 0)
    );
end entity;

architecture behav of MUX_{0} is
begin
sel_process : process(SEL, DIN)
    begin
    case SEL is
""".format(muxName, str(selWidth), str(inWidth-1), str(dataWidth-1)))

    for i in range(inWidth):
        outFile.write("        when \""  + bin(i)[2:].zfill(selWidth) + "\" => DOUT <= DIN(" + str(i) + ");\n")
    outFile.write("""        when others => DOUT <= (others => {0});
    end case;
end process;
end architecture;
""".format(default))

    outFile.close()

if __name__ == "__main__":
    genBigMUX(25, "AU_IN")
    genBigMUX(9, "BRAM_IN")
from math import *
import genMUX
import genMUX
import json

def genLUDHardware(confFileName, generateHILA = True):
    vhdFile  = open("../autoHdlFiles/LUDHardware.vhd", 'w')

    hwConf = json.load(open(confFileName, 'r'))
    bramNames = hwConf["hwConfig"]["bram_names"]
    macNames = hwConf["hwConfig"]["mac_names"]
    divNames = hwConf["hwConfig"]["div_names"]


    # bramNames = ['A', 'B', 'C', 'D']
    # macNames = ['A', 'B', 'C', 'D']
    # divNames = ['A', 'B', 'C', 'D']
    # bramNames = ['A']
    # macNames = ['A']
    # divNames = ['A']

    auSelSize = hwConf["hwConfig"]["auMuxSelWidth"]
    bramSelSize = hwConf["hwConfig"]["bramMuxSelWidth"]
    bramAddrSize = hwConf["hwConfig"]["bramAddrWidth"]
    # auSelSize = ceil(log2(1 + len(macNames) + len(divNames) + 4*len(bramNames)))
    # bramSelSize = ceil(log2(1 + len(macNames) + len(divNames)))

    # genMUX.genBigMUX(auSelSize, "AU_IN")
    # genMUX.genBigMUX(bramSelSize, "BRAM_IN")

    ctrlSignalSize = 3 * auSelSize * len(macNames) + 2 * auSelSize * len(divNames) + 4 * bramSelSize * len(bramNames) + (bramAddrSize + 1) * 4 * len(bramNames)

    genTypesPackage()

    vhdFile.write("""
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library work;
use work.all;
use work.types.all;

entity LUDHardware is
    generic(
        ADDR_WIDTH : integer := {2};
        CTRL_WIDTH : integer := {1}
    );
    port(
        CLK_IN : in std_logic;
        RST_IN : in std_logic;
        CTRL_Signal : in std_logic_vector(CTRL_WIDTH-1 downto 0);
        CTRL_VALID  : in std_logic;
        COMPLETED : in std_logic;
        CLK_OUT : out std_logic;
        RST_OUT : out std_logic
    );
end entity;

architecture yoStyle of LUDHardware is

    signal CLK_100, CLK_200, CLK_100_gated : std_logic;
    signal RST : std_logic;
    signal locked : std_logic;

    signal inputLocations : dataArray({0} downto 0);

""".format(str(len(macNames) + len(divNames) + 4*len(bramNames)), str(ctrlSignalSize), str(bramAddrSize)))

    ############## generate signals section

    for bramName in bramNames:
        genQuadBRAMSignals(bramName, vhdFile)

    for macName in macNames:
        genMACSignals(macName, vhdFile)

    for divName in divNames:
        genDivSignals(divName, vhdFile)

    genSelSignals(macNames, divNames, bramNames, vhdFile)

    ############## connect section

    vhdFile.write("""
    
begin

    """)

    for bramName in bramNames:
        genQuadBRAMConnect(bramName, vhdFile)

    for macName in macNames:
        genMACConnect(macName, vhdFile)

    for divName in divNames:
        genDivConnect(divName, vhdFile)

    genAUMuxConnect(macNames, divNames, bramNames, vhdFile)
    genBRAMMUXConnect(macNames, divNames, bramNames, vhdFile)

    genSelConnect(macNames, divNames, bramNames, auSelSize, bramSelSize, ctrlSignalSize, vhdFile)
    genBRAMENconnect(bramNames, vhdFile)
    genCLKConnect(vhdFile)
    genConstantConnect(macNames, divNames, vhdFile)

    if generateHILA:
        genHILAConnect(bramNames, vhdFile)

    vhdFile.write("\nend architecture;\n")
    vhdFile.close()


    auMuxIns = 1 + 4 * len(bramNames) + len(macNames) + len(divNames)
    bramMuxIns = 1 + len(macNames) + len(divNames)
    genMUX.genBigMUX(auMuxIns, "AU_IN")
    genMUX.genBigMUX(bramMuxIns, "BRAM_IN")



############# BRAM Related #########################################


def genQuadBRAMSignals(bramName,outFile):
    singlePortSignalsStr = """
    signal {0}_port{1}_addr : std_logic_vector(ADDR_WIDTH-1 downto 0);
    signal {0}_port{1}_din  : std_logic_vector(31 downto 0);
    signal {0}_port{1}_dout : std_logic_vector(31 downto 0);
    signal {0}_port{1}_en   : std_logic;
    signal {0}_port{1}_we   : std_logic;
    """
    ## Block connection ports
    outFile.write(singlePortSignalsStr.format("block_" + bramName, 'a'))
    outFile.write(singlePortSignalsStr.format("block_" + bramName, 'b'))

    ## PORT singals
    outFile.write(singlePortSignalsStr.format('bram_' + bramName, 'a'))
    outFile.write(singlePortSignalsStr.format('bram_' + bramName, 'b'))
    outFile.write(singlePortSignalsStr.format('bram_' + bramName, 'c'))
    outFile.write(singlePortSignalsStr.format('bram_' + bramName, 'd'))


def genQuadBRAMConnect(bramName, outFile):
    outFile.write("""BRAM_{0} : entity quadPortBRAM
    generic map(10, 32)
    port MAP(
        CLK_1X          => CLK_100,
        CLK_2X          => CLK_200,
        RST             => RST,
""".format(bramName))
    # connect ports
    singleQuadPortConnection = """
        {0}_PORT{1}_ADDR    => {2}_port{3}_addr,
        {0}_PORT{1}_DIN     => {2}_port{3}_din,
        {0}_PORT{1}_DOUT    => {2}_port{3}_dout,
        {0}_PORT{1}_EN      => {2}_port{3}_en,
        {0}_PORT{1}_WE      => {2}_port{3}_we,
    """
    outFile.write(singleQuadPortConnection.format("BRAM", 'A', 'bram_' + bramName, 'a'))
    outFile.write(singleQuadPortConnection.format("BRAM", 'B', 'bram_' + bramName, 'b'))
    outFile.write(singleQuadPortConnection.format("BRAM", 'C', 'bram_' + bramName, 'c'))
    outFile.write(singleQuadPortConnection.format("BRAM", 'D', 'bram_' + bramName, 'd'))

    blockPortString = """
        BLOCK_PORTA_ADDR    => block_{0}_porta_addr,
        BLOCK_PORTA_DIN     => block_{0}_porta_din,
        BLOCK_PORTA_DOUT    => block_{0}_porta_dout,
        BLOCK_PORTA_EN      => block_{0}_porta_en,
        BLOCK_PORTA_WE      => block_{0}_porta_we,

        BLOCK_PORTB_ADDR    => block_{0}_portb_addr,
        BLOCK_PORTB_DIN     => block_{0}_portb_din,
        BLOCK_PORTB_DOUT    => block_{0}_portb_dout,
        BLOCK_PORTB_EN      => block_{0}_portb_en,
        BLOCK_PORTB_WE      => block_{0}_portb_we
    );

    """

    outFile.write(blockPortString.format(bramName))

    blockConnectionString = """
    block_{0} : entity design_BRAM_{0}_wrapper
    port map (
        BRAM_PORTA_addr     => block_{0}_porta_addr,
        BRAM_PORTA_clk      => CLK_200,
        BRAM_PORTA_din      => block_{0}_porta_din,
        BRAM_PORTA_dout     => block_{0}_porta_dout,
        BRAM_PORTA_en       => block_{0}_porta_en,
        BRAM_PORTA_we(0)    => block_{0}_porta_we,
        BRAM_PORTB_addr     => block_{0}_portb_addr,
        BRAM_PORTB_clk      => CLK_200,
        BRAM_PORTB_din      => block_{0}_portb_din,
        BRAM_PORTB_dout     => block_{0}_portb_dout,
        BRAM_PORTB_en       => block_{0}_portb_en,
        BRAM_PORTB_we(0)    => block_{0}_portb_we
    );

    """

    outFile.write(blockConnectionString.format(bramName))

############# PE Related #################################

def genMACSignals(macName, outFile):
    signlePortSignals = """
    signal mac_{0}_{1}_tdata : std_logic_vector({2} downto 0);
    signal mac_{0}_{1}_tready : std_logic;
    signal mac_{0}_{1}_tvalid : std_logic;
    """

    outFile.write(signlePortSignals.format(macName, 'result', '31'))
    outFile.write(signlePortSignals.format(macName, 'a', '31'))
    outFile.write(signlePortSignals.format(macName, 'b', '31'))
    outFile.write(signlePortSignals.format(macName, 'c', '31'))
    outFile.write(signlePortSignals.format(macName, 'operation', '7')) 

    outFile.write("signal mac_{0}_a_signInv : std_logic;\n".format(macName))
    outFile.write("    signal mac_{0}_a_tdataIN : std_logic_vector(31 downto 0);\n".format(macName))

def genDivSignals(divName, outFile):
    signlePortSignals = """
    signal div_{0}_{1}_tdata : std_logic_vector(31 downto 0);
    signal div_{0}_{1}_tready : std_logic;
    signal div_{0}_{1}_tvalid : std_logic;
    """

    outFile.write(signlePortSignals.format(divName, 'result'))
    outFile.write(signlePortSignals.format(divName, 'a'))
    outFile.write(signlePortSignals.format(divName, 'b'))


def genMACConnect(macName, outFile):
    outFile.write("""
    MAC_{0} : entity design_MAC_wrapper
    port map(
""".format(macName))

    for (MACPort, mode, sysPort) in [('RESULT', 'M', 'result'), ('A', 'S', 'a'), ('B', 'S', 'b'), ('C', 'S', 'c'), ('OPERATION', 'S', 'operation')]:
        outFile.write("""        {0}_AXIS_{1}_tdata => mac_{2}_{3}_tdata,
        {0}_AXIS_{1}_tready => mac_{2}_{3}_tready,
        {0}_AXIS_{1}_tvalid => mac_{2}_{3}_tvalid,
""".format(mode, MACPort, macName, sysPort))
    outFile.write("""        aclk   => CLK_100_gated
    );
""")

def genDivConnect(divName, outFile):
    outFile.write("""
    DIV_{0} : entity design_DIV_wrapper
    port map(
""".format(divName))

    for (DivPort, mode, sysPort) in [('RESULT', 'M', 'result'), ('A', 'S', 'a'), ('B', 'S', 'b')]:
        outFile.write("""        {0}_AXIS_{1}_tdata => div_{2}_{3}_tdata,
        {0}_AXIS_{1}_tready => div_{2}_{3}_tready,
        {0}_AXIS_{1}_tvalid => div_{2}_{3}_tvalid,
""".format(mode, DivPort, divName, sysPort))
    outFile.write("""        aclk   => CLK_100_gated
    );
""")


################ MUX Related ########################

def genAUMuxConnect(macNames, divNames, BRAMNames, outFile):
    # connect MAC out locations
    outFile.write("\n -- input locations \n")
    for i in range(len(macNames)):
        outFile.write("    inputLocations({0}) <= mac_{1}_result_tdata;\n".format(str(i), macNames[i]))
    for i in range(len(divNames)):
        outFile.write("    inputLocations({0}) <= div_{1}_result_tdata;\n".format(str(i + len(macNames)), macNames[i]))
    for i in range(len(BRAMNames)):
        outFile.write("""    inputLocations({0}) <= bram_{1}_porta_dout;
    inputLocations({0}+1) <= bram_{1}_portb_dout;
    inputLocations({0}+2) <= bram_{1}_portc_dout;
    inputLocations({0}+3) <= bram_{1}_portd_dout;
""".format(str(4*i+len(macNames) + len(divNames)), macNames[i]))
    outFile.write("    inputLocations({0}) <= (others => '0');\n".format(str(len(macNames) + len(divNames) + 4*len(BRAMNames))))
    ### AU Port In Muxes
    for macName in macNames:
        for port in ['b', 'c']:
            outFile.write("""
    MUX_MAC_IN_{0}_{1} : entity MUX_AU_IN 
    port map(
        SEL => mac_{0}_{1}_sel,
        DIN => inputLocations,
        DOUT => mac_{0}_{1}_tdata
    );
""".format(macName, port))
        outFile.write("""
    MUX_MAC_IN_{0}_a : entity MUX_AU_IN 
    port map(
        SEL => mac_{0}_a_sel,
        DIN => inputLocations,
        DOUT => mac_{0}_a_tdataIN
    );

    mac_{0}_a_signInv <= not mac_{0}_a_tdataIN(31);
    mac_{0}_a_tdata <= mac_{0}_a_signInv & mac_{0}_a_tdataIN(30 downto 0);
""".format(macName, ))

    for divName in divNames:
        for port in ['a', 'b']:
            outFile.write("""
    MUX_DIV_IN_{0}_{1} : entity MUX_AU_IN 
    port map(
        SEL => div_{0}_{1}_sel,
        DIN => inputLocations,
        DOUT => div_{0}_{1}_tdata
    );
""".format(divName, port))


def genBRAMMUXConnect(macNames, divNames, BRAMNames, outFile):
    for bram in BRAMNames:
        for port in ['a', 'b', 'c', 'd']:
            outFile.write("""
    MUX_BRAM_{0}_{1}_in : entity MUX_BRAM_IN
    port map(
        SEL => bram_{0}_{1}_sel,
        DIN => inputLocations({2} downto 0),
        DOUT => bram_{0}_port{1}_din
    );
""".format(bram, port, str(len(macNames) + len(divNames))))


###### Control signals ###############################

def genSelSignals(macNames, divNames, bramNames, outFile):
    auSelSize = ceil(log2(1 + len(macNames) + len(divNames) + 4*len(bramNames)))
    bramSelSize = ceil(log2(1 + len(macNames) + len(divNames)))
    
    for macName in macNames:
        outFile.write("\n    signal mac_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(macName, 'a', str(auSelSize)))
        outFile.write("    signal mac_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(macName, 'b', str(auSelSize)))
        outFile.write("    signal mac_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(macName, 'c', str(auSelSize)))
    for divName in divNames:
        outFile.write("\n    signal div_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(divName, 'a', str(auSelSize)))
        outFile.write("    signal div_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(divName, 'b', str(auSelSize)))
    for bramName in bramNames:
        outFile.write("\n    signal bram_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(bramName, 'a', str(bramSelSize)))
        outFile.write("    signal bram_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(bramName, 'b', str(bramSelSize)))
        outFile.write("    signal bram_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(bramName, 'c', str(bramSelSize)))
        outFile.write("    signal bram_{0}_{1}_sel : std_logic_vector({2}-1 downto 0);\n".format(bramName, 'd', str(bramSelSize)))

def genSelConnect(macNames, divNames, bramNames, auSelSize, bramSelSize, ctrlSize, outFile):
    outFile.write("\n")
    currentCtrlStart = ctrlSize - 1

    for bram in bramNames:
        for port in ['a', 'b', 'c', 'd']:
            outFile.write("    bram_{0}_port{1}_addr <= CTRL_Signal({2} downto {3});\n".format(bram, port, str(currentCtrlStart), str(currentCtrlStart-9)))
            currentCtrlStart -= 10
            outFile.write("    bram_{0}_port{1}_we <= CTRL_Signal({2});\n".format(bram, port, str(currentCtrlStart)))
            currentCtrlStart -=1

    for mac in macNames:
        for port in ['a', 'b', 'c']:
            outFile.write("    mac_{0}_{1}_sel <= CTRL_Signal({2} downto {3});\n".format(mac, port, str(currentCtrlStart), str(currentCtrlStart-auSelSize+1)))
            currentCtrlStart -= auSelSize

    for div in divNames:
        for port in ['a', 'b']:
            outFile.write("    div_{0}_{1}_sel <= CTRL_Signal({2} downto {3});\n".format(div, port, str(currentCtrlStart), str(currentCtrlStart-auSelSize+1)))
            currentCtrlStart -= auSelSize

    for bram in bramNames:
        for port in ['a', 'b', 'c', 'd']:
            outFile.write("    bram_{0}_{1}_sel <= CTRL_Signal({2} downto {3});\n".format(bram, port, str(currentCtrlStart), str(currentCtrlStart-bramSelSize+1)))
            currentCtrlStart -= bramSelSize

def genBRAMENconnect(bramNames, outFile):
    for bram in bramNames:
        for port in ['a', 'b', 'c', 'd']:
            outFile.write("    bram_{0}_port{1}_en <= CTRL_VALID and locked;\n".format(bram, port))


############# other stuff ##########

# def genCLKSignals(outFile):
#     outFile.write("""
#     signal 
# """)

def genCLKConnect(outFile):
    outFile.write("""
    CLK_RST : entity design_CLK_RST_wrapper
    port map(
        clk_in1     => CLK_IN,
        clk_out1    => CLK_100,
        clk_out2    => CLK_200,
        locked      => locked,
        reset       => RST_IN
    );

    CLK_OUT <= CLK_100;
    RST_OUT <= RST;
    RST <= not locked;
    CLK_100_gated <= CLK_100 and CTRL_VALID;
""")


def genConstantConnect(macNames, divNames, outFile):
    outFile.write("\n")
    for mac in macNames:
        for port in ['a', 'b', 'c', 'operation']:
            outFile.write("    mac_{0}_{1}_tvalid <= locked;\n".format(mac, port))
        outFile.write("    mac_{0}_operation_tdata <= \"00000000\";\n".format(mac))
        outFile.write("    mac_{0}_result_tready <= locked;\n".format(mac))
    
    outFile.write("\n")
    for div in divNames:
        for port in ['a', 'b']:
            outFile.write("    div_{0}_{1}_tvalid <= locked;\n".format(div, port))
        outFile.write("    div_{0}_result_tready <= locked;\n".format(div))

def genTypesPackage(fileName = "../autoHdlFiles/types.vhd"):
    outFile = open(fileName, 'w')
    outFile.write("""
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package types is

    type dataArray is array(natural range <>) of std_logic_vector(31 downto 0);

end package;
""")
    outFile.close()


##################  ILA ####################################


# def genHILASignals(bramNames, outFile):
#     outFile.write("\n")
#     for i in range(1, len(bramNames) * 4 + 1):
#         outFile.write("    signal hilaProbe{0} : std_logic_vector(ADDR_WIDTH + 32 downto 0);\n".format(str(i)))


def genHILAConnect(bramNames, outFile):
    outFile.write("""
    hila : entity design_HILA_wrapper
    port map (
        clk => CLK_100,
        probe0(0) => CTRL_VALID,
""")
    probeCount = 1
    for bram in bramNames:
        for port in ['a', 'b', 'c', 'd']:
            outFile.write("        probe{0}(0) => bram_{1}_port{2}_we,\n".format(str(probeCount), bram, port))
            probeCount += 1
            outFile.write("        probe{0} => bram_{1}_port{2}_addr,\n".format(str(probeCount), bram, port))
            probeCount += 1
            outFile.write("        probe{0} => bram_{1}_port{2}_din,\n".format(str(probeCount), bram, port))
            probeCount += 1
            outFile.write("        probe{0} => bram_{1}_port{2}_dout,\n".format(str(probeCount), bram, port))
            probeCount += 1
    
    outFile.write("""        probe{0}(0) => COMPLETED
    );
""".format(str(probeCount)))
    outFile.write("\n")




if __name__ == "__main__":
    genLUDHardware("../confFiles/hwConfig.json")
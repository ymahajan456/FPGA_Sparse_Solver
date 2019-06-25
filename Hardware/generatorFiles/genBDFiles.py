import os, json

####### BRAM Files #########
dataBRAMGenStr = """
## BRAM {0}

create_bd_design "design_BRAM_{0}"
create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.3 blk_mem_gen_0
set_property -dict [list CONFIG.Memory_Type {{True_Dual_Port_RAM}} CONFIG.Write_Depth_A {{1024}} CONFIG.Register_PortA_Output_of_Memory_Primitives {{false}} CONFIG.Register_PortB_Output_of_Memory_Primitives {{false}} CONFIG.Load_Init_File {{true}} CONFIG.Coe_File {{{1}}} CONFIG.use_bram_block {{Stand_Alone}} CONFIG.Enable_32bit_Address {{false}} CONFIG.Use_Byte_Write_Enable {{false}} CONFIG.Byte_Size {{9}} CONFIG.Enable_B {{Use_ENB_Pin}} CONFIG.Use_RSTA_Pin {{false}} CONFIG.Port_B_Clock {{100}} CONFIG.Port_B_Write_Rate {{50}} CONFIG.Port_B_Enable_Rate {{100}}] [get_bd_cells blk_mem_gen_0]
make_bd_pins_external  [get_bd_cells blk_mem_gen_0]
make_bd_intf_pins_external  [get_bd_cells blk_mem_gen_0]
save_bd_design

close_bd_design [get_bd_designs design_BRAM_{0}]


"""

updateBRAMGenStr = """
## BRAM {0}

create_bd_design "design_BRAM_{0}"
set_property -dict [list CONFIG.Memory_Type {{True_Dual_Port_RAM}} CONFIG.Write_Depth_A {{1024}} CONFIG.Register_PortA_Output_of_Memory_Primitives {{false}} CONFIG.Register_PortB_Output_of_Memory_Primitives {{false}} CONFIG.Load_Init_File {{true}} CONFIG.Coe_File {{{1}}} CONFIG.use_bram_block {{Stand_Alone}} CONFIG.Enable_32bit_Address {{false}} CONFIG.Use_Byte_Write_Enable {{false}} CONFIG.Byte_Size {{9}} CONFIG.Enable_B {{Use_ENB_Pin}} CONFIG.Use_RSTA_Pin {{false}} CONFIG.Port_B_Clock {{100}} CONFIG.Port_B_Write_Rate {{50}} CONFIG.Port_B_Enable_Rate {{100}}] [get_bd_cells blk_mem_gen_0]
save_bd_design


"""

simClkRst = """

create_bd_design "design_simClkRst"
create_bd_cell -type ip -vlnv xilinx.com:ip:sim_clk_gen:1.0 sim_clk_gen_0
set_property -dict [list CONFIG.CLOCK_TYPE {Single_Ended} CONFIG.RESET_POLARITY {ACTIVE_HIGH} CONFIG.FREQ_HZ {125000000}] [get_bd_cells sim_clk_gen_0]
make_bd_pins_external  [get_bd_cells sim_clk_gen_0]
make_bd_intf_pins_external  [get_bd_cells sim_clk_gen_0]
save_bd_design

close_bd_design [get_bd_designs design_simClkRst]


"""

clockRstGenStr = """
### CLK_RST

create_bd_design "design_CLK_RST"
create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:5.4 clk_wiz_0
set_property -dict [list CONFIG.CLK_IN1_BOARD_INTERFACE {sys_clock} CONFIG.CLKOUT2_USED {true} CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {200.000} CONFIG.CLKIN1_JITTER_PS {80.0} CONFIG.MMCM_DIVCLK_DIVIDE {1} CONFIG.MMCM_CLKFBOUT_MULT_F {8.000} CONFIG.MMCM_CLKIN1_PERIOD {8.000} CONFIG.MMCM_CLKOUT1_DIVIDE {5} CONFIG.NUM_OUT_CLKS {2} CONFIG.CLKOUT1_JITTER {124.615} CONFIG.CLKOUT1_PHASE_ERROR {96.948} CONFIG.CLKOUT2_JITTER {109.241} CONFIG.CLKOUT2_PHASE_ERROR {96.948}] [get_bd_cells clk_wiz_0]
make_bd_pins_external  [get_bd_cells clk_wiz_0]
make_bd_intf_pins_external  [get_bd_cells clk_wiz_0]
save_bd_design

close_bd_design [get_bd_designs design_CLK_RST]


"""

PEGenStr = """


## MAC Unit
create_bd_design "design_MAC"
create_bd_cell -type ip -vlnv xilinx.com:ip:floating_point:7.1 floating_point_0
set_property -dict [list CONFIG.Operation_Type {FMA} CONFIG.C_Mult_Usage {Full_Usage} CONFIG.Maximum_Latency {false} CONFIG.C_Latency {3} CONFIG.Result_Precision_Type {Single} CONFIG.C_Result_Exponent_Width {8} CONFIG.C_Result_Fraction_Width {24} CONFIG.C_Rate {1}] [get_bd_cells floating_point_0]
make_bd_pins_external  [get_bd_cells floating_point_0]
make_bd_intf_pins_external  [get_bd_cells floating_point_0]
save_bd_design

## DIV Unit
create_bd_design "design_DIV"
create_bd_cell -type ip -vlnv xilinx.com:ip:floating_point:7.1 floating_point_0
set_property -dict [list CONFIG.Operation_Type {Divide} CONFIG.Flow_Control {Blocking} CONFIG.Axi_Optimize_Goal {Performance} CONFIG.Maximum_Latency {false} CONFIG.C_Latency {3} CONFIG.Result_Precision_Type {Single} CONFIG.C_Result_Exponent_Width {8} CONFIG.C_Result_Fraction_Width {24} CONFIG.C_Mult_Usage {No_Usage} CONFIG.Has_RESULT_TREADY {true} CONFIG.C_Rate {1}] [get_bd_cells floating_point_0]
make_bd_pins_external  [get_bd_cells floating_point_0]
make_bd_intf_pins_external  [get_bd_cells floating_point_0]
save_bd_design

close_bd_design [get_bd_designs design_MAC]
close_bd_design [get_bd_designs design_DIV]


"""

def genHILABDFile(numBrams, numPorts, tclFileName):
    tclFile = open(tclFileName, 'a')
    tclFile.write("""

create_bd_design "design_HILA"
create_bd_cell -type ip -vlnv xilinx.com:ip:ila:6.2 ila_0
set_property -dict [list CONFIG.C_NUM_OF_PROBES {{{0}}} CONFIG.C_MONITOR_TYPE {{Native}} CONFIG.C_ENABLE_ILA_AXI_MON {{false}}] [get_bd_cells ila_0]
""".format(str(numBrams * numPorts * 4 + 2)))
    j = 1
    for i in range(numBrams * numPorts):
        tclFile.write("set_property -dict [list CONFIG.C_PROBE{0}_TYPE {{1}} CONFIG.C_PROBE{0}_WIDTH {{{1}}}] [get_bd_cells ila_0]\n".format(str(j), "1"))
        j += 1
        tclFile.write("set_property -dict [list CONFIG.C_PROBE{0}_TYPE {{1}} CONFIG.C_PROBE{0}_WIDTH {{{1}}}] [get_bd_cells ila_0]\n".format(str(j), "10"))
        j += 1
        tclFile.write("set_property -dict [list CONFIG.C_PROBE{0}_TYPE {{1}} CONFIG.C_PROBE{0}_WIDTH {{{1}}}] [get_bd_cells ila_0]\n".format(str(j), "32"))
        j += 1
        tclFile.write("set_property -dict [list CONFIG.C_PROBE{0}_TYPE {{1}} CONFIG.C_PROBE{0}_WIDTH {{{1}}}] [get_bd_cells ila_0]\n".format(str(j), "32"))
        j += 1
    tclFile.write("""make_bd_pins_external  [get_bd_cells ila_0]
make_bd_intf_pins_external  [get_bd_cells ila_0]
save_bd_design

close_bd_design [get_bd_designs design_HILA]


""")
    tclFile.close()


def genTesterBRAMBdFile(ctrlWidth , tclFileName, initFileName):
    tclFile = open(tclFileName, 'a')
    tclFile.write("""

create_bd_design "design_CTRL"
create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.3 blk_mem_gen_0
set_property -dict [list CONFIG.Write_Width_A {{{0}}} CONFIG.Write_Depth_A {{512}} CONFIG.Register_PortA_Output_of_Memory_Primitives {{false}} CONFIG.Load_Init_File {{true}} CONFIG.Coe_File {{{1}}} CONFIG.use_bram_block {{Stand_Alone}} CONFIG.Enable_32bit_Address {{false}} CONFIG.Use_Byte_Write_Enable {{false}} CONFIG.Byte_Size {{9}} CONFIG.Read_Width_A {{{0}}} CONFIG.Write_Width_B {{{0}}} CONFIG.Read_Width_B {{{0}}} CONFIG.Use_RSTA_Pin {{false}}] [get_bd_cells blk_mem_gen_0]
make_bd_pins_external  [get_bd_cells blk_mem_gen_0]
make_bd_intf_pins_external  [get_bd_cells blk_mem_gen_0]
save_bd_design
close_bd_design [get_bd_designs design_CTRL]


""".format(str(ctrlWidth), initFileName))
    tclFile.close()

def genOtherBDFiles(coeFileMap, tclFileName):
    """
    coeFileMap : directory of data BRAM names and coefficient file address
            as key value pair
    """
    bdList = []
    tclFile = open(tclFileName, 'w')
    for BRAMName in coeFileMap.keys():
        tclFile.write(dataBRAMGenStr.format(BRAMName, coeFileMap[BRAMName]))
        bdList.append("design_BRAM_" + BRAMName)
    ## Clock and Reset Handler
    tclFile.write(clockRstGenStr)
    bdList.append("design_CLK_RST")
    ## PE
    tclFile.write(PEGenStr)
    bdList.append("design_MAC")
    bdList.append("design_DIV")
    ## simulation clock and reset source
    tclFile.write(simClkRst)
    bdList.append("design_simClkRst")
    tclFile.close()
    return(bdList)

def genAllBDFiles(confFileName, tclFileName, bdfileList):
    hwConf = json.load(open(confFileName, 'r'))
    
    bramNames = hwConf["hwConfig"]["bram_names"]
    macNames = hwConf["hwConfig"]["mac_names"]
    divNames = hwConf["hwConfig"]["div_names"]
    auSelSize = hwConf["hwConfig"]["auMuxSelWidth"]
    bramSelSize = hwConf["hwConfig"]["bramMuxSelWidth"]
    bramAddrSize = hwConf["hwConfig"]["bramAddrWidth"]
    ctrlSignalSize = 3 * auSelSize * len(macNames) + 2 * auSelSize * len(divNames) + 4 * bramSelSize * len(bramNames) + (bramAddrSize + 1) * 4 * len(bramNames)
    
    coeFileMap = {}
    for bram in bramNames:
        coeFileMap[bram] = os.path.abspath('../testFiles/bram_' + bram + '.coe')
    bdList = genOtherBDFiles(coeFileMap, tclFileName)
    genHILABDFile(len(bramNames), 4, tclFileName)
    bdList.append("design_HILA")
    testerCoeFileName = os.path.abspath("../testFiles/instructionStream.coe")
    genTesterBRAMBdFile(ctrlSignalSize, tclFileName, testerCoeFileName)
    bdList.append("design_CTRL")

    bdListObj = open(bdfileList, 'w')
    for bd in bdList:
        bdListObj.write(bd + '\n')

if __name__ == "__main__":
    genAllBDFiles("../confFiles/hwConfig.json", "../tclScripts/genBDFiles.tcl", '../confFiles/bdList.conf')
import json
import os, math

def genHwConfigStruct(name, numPEs, numBRAMs, numPorts, latencyBRAM, latencyMAC, latencyDiv, outFile):
    outFile.write("""
    string postfix_{0} = "_A{1}_B{2}_P{3}_DB{4}_DM{5}_DD{6}";
""".format(name, str(numPEs), str(numBRAMs), str(numPorts), str(latencyBRAM), str(latencyMAC), str(latencyDiv)))
    
    outFile.write("""
    struct BRAM bramConfig_{0} = 
    {{10000, {1}, {2}, 0}};

    struct ArithmeticUnit macConfig_{0}= {{
        5,
        {{"*","mac_add", "mac_sub", "+", "-"}},
        true,
        {3},
        {{20,20,20,20,20}}
    }};

    struct ArithmeticUnit divConfig_{0} = {{
        1,
        {{"/"}},
        true,
        {4},
        {{29}}
    }};

    struct HardwareConfig hardwareConfig_{0} =
    {{
        {{
""".format(name, str(numPorts), str(latencyBRAM), str(latencyMAC), str(latencyDiv)))

    for bram in range(numBRAMs-1):
        outFile.write("            &bramConfig_{0},\n".format(name))
    outFile.write("            &bramConfig_{0}\n        }},\n        {{\n".format(name))
    
    for mac in range(numPEs):
        outFile.write("            &macConfig_{0},\n".format(name))
    
    for div in range(numPEs-1):
        outFile.write("            &divConfig_{0},\n".format(name))
    outFile.write("            &divConfig_{0}\n        }},\n        {{".format(name))

    bramName = 'A'
    for bram in range(numBRAMs-1):
        outFile.write("\"" + bramName + "\", ")
        bramName = chr(ord(bramName) +1)
    outFile.write("\"{0}\"}},\n        10,".format(bramName))
    
    auSelWidth = math.ceil(math.log2(1 + numBRAMs * numPorts + 2 * numPEs))
    bramSelWidth = math.ceil(math.log2(1 + 2 * numPEs))

    outFile.write("""
        {0},
        {1},
        {2},
        0
    }};

""".format(str(auSelWidth), str(bramSelWidth), str(numPEs)))




def genMultipleTestsMainFile(jsonFileName):
    outFile  = open("../cppFiles/mainMultipleTests.cpp", 'w')
    outFile.write("""
#define DEFINE_VARIABLES
#include "stdafx.h"
#include "CCS.h"
#include "Graph.h"
#include "sparseLU.h"
#include "Schedule.h"
#include "Hardware.h"

void test(struct HardwareConfig &simpleHardware, string testName = "rajat11", string postfix = "");

int main(int argc, char* argv[])
{
#ifdef PRINT_DEBUG_MSG
    debugFile.open("debugMessage.txt");
#else
    ofstream debugFile("Output.log");
#endif

    g_nzThreshold = 1.0e-7;
    g_errThreshold = 1.0e-3;

""")

    tests = json.load(open(jsonFileName, 'r'))["HW_CONFIGS"]

    i = 0
    for test in tests:
        genHwConfigStruct(str(i), test["NUM_PES"], test["NUM_BRAMS"], test["NUM_PORTS"], test["LATENCY_BRAM"], test["LATENCY_MAC"], test["LATENCY_DIV"], outFile)
        i += 1

    outFile.write("""
    cout << "Checking if processor is available...";
    if(system(NULL)) puts("ok");
    else exit(EXIT_FAILURE);

    int retVal = system("mkdir -p ../outputFiles");
    if(retVal == -1)
    {
        cout << "🚨 🚨Error: Can't create outFiles directory. Bailing out!" << endl;
        exit(0);
    }
""")

    testMatrices = ["rajat11", 'fpga_dcop_01', 'fpga_dcop_02', 'fpga_dcop_03', 'fpga_dcop_04', 'fpga_dcop_05']

    for i in range(len(tests)):
        for mat in testMatrices:
            outFile.write("    test(hardwareConfig_{0}, \"{1}\", postfix_{0});\n".format(str(i), mat))

    outFile.write("""

    debugFile.close();

    cout << "Exiting Main" << endl;
}





void test(struct HardwareConfig &simpleHardware, string testName,  string postfix)
{
    string dotFileName = "exeTree";
    // test matrix name


    CCS A("../test/exported/" + testName + "/" + testName +"_amd_ALU", "A");
    CCS L(A.numRows, A.numCols, "L");
    CCS U(A.numRows, A.numCols, "U");

    cout << "Cols: " << A.numCols << " Rows: " << A.numCols << endl;

    Graph LUmat;
    debugFile << "Main: Init completed" << endl;
    LUGetOperationList(A, L, U, LUmat);
    A.resizeMemMap();
    L.resizeMemMap();
    U.resizeMemMap();
    LUmat.removeUselessNodes();
    LUmat.findLeavesAndRoots();
    LUmat.assignLevelsAndIds();
    assignPriority(LUmat);
    LUmat.executeGraphUsingLevels(dotFileName);
    assignGraphValuesToMats(LUmat, L, U);

    // LUmat.printToFile(debugFile, true);
    // L.print();
    // U.print();

    CCS Uref("../test/exported/" + testName + "/" + testName + "_amd_U", "Uref");
    CCS Lref("../test/exported/" + testName + "/" + testName + + "_amd_L", "Lref");

    CCS Ucpy = U;
    CCS Lcpy = L;

    cout << "Comparing U Matrix: " << ((Ucpy == Uref) ? "1 ✅" : "0 ❌") << endl;
    cout << "Comparing L Matrix: " << ((Lcpy == Lref) ? "1 ✅" : "0 ❌") << endl;

    assignScatteredAddresses(LUmat, A, L, U, simpleHardware);
    Schedule schedule;
    scheduleNoReg(LUmat, simpleHardware, schedule);

    convertScheduleToInstructionStream(simpleHardware, schedule);
    A.generateBRAMCoeFiles(simpleHardware.bramNames);

    exportHardwareConfig(simpleHardware);

    cout << "Cycles: " << schedule.AUSchedule.size() << endl;

    ofstream resultFile("../test/" + testName + postfix + "_log.csv");
    resultFile << "Schedule for " << testName << endl;
    resultFile << "clock, Scheduled Flops" << endl; 
    for(int i = 0; i < schedule.AUSchedule.size(); i++)
    {
        resultFile << to_string(i) + ", " + to_string(schedule.AUSchedule[i].size()) << endl;
    }
    resultFile.close();

    // cout << "Aab Kya Pain Hai?" << endl;

    // U.printToFile("U");
    // Uref.printToFile("Uref");
    // L.printToFile("L");
    // Lref.printToFile("Lref");

    // L.print();
    // Lref.print();
    // U.print();
    // Uref.print();

    /* ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠
    * Takes tooooooooooo much time. Don't ever uncomment
    */
    // #ifdef MAKE_GRAPH
    // convertDotToPng(dotFileName);
    // #endif
}
""")


    outFile.close()



if __name__ == "__main__":
    jsonFileName = "../test/testConfig.json"
    genMultipleTestsMainFile(jsonFileName)
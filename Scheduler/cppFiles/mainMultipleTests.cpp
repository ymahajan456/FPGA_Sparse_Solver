
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


    string postfix_0 = "_A4_B4_P4_DB2_DM3_DD3";

    struct BRAM bramConfig_0 = 
    {10000, 4, 2, 0};

    struct ArithmeticUnit macConfig_0= {
        5,
        {"*","mac_add", "mac_sub", "+", "-"},
        true,
        3,
        {20,20,20,20,20}
    };

    struct ArithmeticUnit divConfig_0 = {
        1,
        {"/"},
        true,
        3,
        {29}
    };

    struct HardwareConfig hardwareConfig_0 =
    {
        {
            &bramConfig_0,
            &bramConfig_0,
            &bramConfig_0,
            &bramConfig_0
        },
        {
            &macConfig_0,
            &macConfig_0,
            &macConfig_0,
            &macConfig_0,
            &divConfig_0,
            &divConfig_0,
            &divConfig_0,
            &divConfig_0
        },
        {"A", "B", "C", "D"},
        10,
        5,
        4,
        4,
        0
    };


    string postfix_1 = "_A4_B4_P4_DB2_DM3_DD3";

    struct BRAM bramConfig_1 = 
    {10000, 4, 2, 0};

    struct ArithmeticUnit macConfig_1= {
        5,
        {"*","mac_add", "mac_sub", "+", "-"},
        true,
        3,
        {20,20,20,20,20}
    };

    struct ArithmeticUnit divConfig_1 = {
        1,
        {"/"},
        true,
        3,
        {29}
    };

    struct HardwareConfig hardwareConfig_1 =
    {
        {
            &bramConfig_1,
            &bramConfig_1,
            &bramConfig_1,
            &bramConfig_1
        },
        {
            &macConfig_1,
            &macConfig_1,
            &macConfig_1,
            &macConfig_1,
            &divConfig_1,
            &divConfig_1,
            &divConfig_1,
            &divConfig_1
        },
        {"A", "B", "C", "D"},
        10,
        5,
        4,
        4,
        0
    };


    cout << "Checking if processor is available...";
    if(system(NULL)) puts("ok");
    else exit(EXIT_FAILURE);

    int retVal = system("mkdir -p ../outputFiles");
    if(retVal == -1)
    {
        cout << "🚨 🚨Error: Can't create outFiles directory. Bailing out!" << endl;
        exit(0);
    }
    test(hardwareConfig_0, "rajat11", postfix_0);
    test(hardwareConfig_0, "fpga_dcop_01", postfix_0);
    test(hardwareConfig_0, "fpga_dcop_02", postfix_0);
    test(hardwareConfig_0, "fpga_dcop_03", postfix_0);
    test(hardwareConfig_0, "fpga_dcop_04", postfix_0);
    test(hardwareConfig_0, "fpga_dcop_05", postfix_0);
    test(hardwareConfig_1, "rajat11", postfix_1);
    test(hardwareConfig_1, "fpga_dcop_01", postfix_1);
    test(hardwareConfig_1, "fpga_dcop_02", postfix_1);
    test(hardwareConfig_1, "fpga_dcop_03", postfix_1);
    test(hardwareConfig_1, "fpga_dcop_04", postfix_1);
    test(hardwareConfig_1, "fpga_dcop_05", postfix_1);


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

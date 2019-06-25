#define DEFINE_VARIABLES
#include "stdafx.h"
#include "CCS.h"
#include "Graph.h"
#include "sparseLU.h"
#include "Schedule.h"
#include "Hardware.h"

void development(struct HardwareConfig &simpleHardware);
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

    /* ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠
    * IMPORTANT NOTICE : Arithmetic units must follow the order : MAC units and then Divider units
    */
    struct HardwareConfig simpleHardware =
    {
        {
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM, 
            &Xilinx_DualPort_BRAM, 
            &Xilinx_DualPort_BRAM
        },
        {
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit
        },
        {"A", "B", "C", "D"},
        10,
        5,
        4,
        4,
        0
    };

    /* ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠
    * IMPORTANT NOTICE : Arithmetic units must follow the order : MAC units and then Divider units
    */
    struct HardwareConfig simpleHardware_16 =
    {
        {
            &Xilinx_DualPort_BRAM, 
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM, 
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM, 
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM, 
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM,
            &Xilinx_DualPort_BRAM
        },
        {
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_MAC_Unit,
            &Xilinx_Floating_MAC_Unit, 
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit,
            &Xilinx_Floating_Divide_Unit
        },
        {"A", "B", "C", "D","E", "F", "G", "H","I", "J", "K", "L","M", "N", "O", "P"},
        10,
        7,
        6,
        16,
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

    if(argc == 1)
    {
        cout << endl << "🏷 Using default 5X5 matrix" << endl;
        development(simpleHardware);
    }
    else
    {
        test(simpleHardware, argv[1]);
    }

    debugFile.close();

    cout << "Exiting Main" << endl;
}

void development(struct HardwareConfig &simpleHardware)
{
    string dotFileName = "exeTree";

    vector<int> colPtr = { 0, 3, 5, 7, 9, 11 };
    vector<int> rowInd = { 0, 2, 3, 1, 3, 0, 4, 1, 3, 0, 4 };
    vector<float> val = { 5.0, 2.0, 1.0, 4.0, -3.0, -5.0, -2.0, -4.0, -1.0, 6.0, 3.0 };

    CCS A(colPtr, rowInd, val, 5, "A");

    // A.addElement(1, 2, 0);
    // A.addElement(2, 2, 1.0e-12);
    // A.addElement(3, 2, 1.0e-11);
    // B.addElement(2, 1, 1.0e-8);
    // A.print();
    // A.removeZeroes();
    // A.print();

    CCS L(A.numRows, A.numCols, "L");
    CCS U(A.numRows, A.numCols, "U");

    Graph LUmat;
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
    assignScatteredAddresses(LUmat, A, L, U, simpleHardware);

    LUmat.printToFile(debugFile, true);
    A.print(true);
    L.print(true);
    U.print(true);

#ifdef MAKE_GRAPH
    convertDotToPng(dotFileName);
#endif


    Schedule schedule;
    scheduleNoReg(LUmat, simpleHardware, schedule);

    printReadableSchedule(schedule);
    convertScheduleToInstructionStream(simpleHardware, schedule);
    vector<string> bramNames{"A", "B", "C", "D"};
    A.generateBRAMCoeFiles(simpleHardware.bramNames);

    exportHardwareConfig(simpleHardware);

    cout << "Cycles: " << schedule.AUSchedule.size() << endl;

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

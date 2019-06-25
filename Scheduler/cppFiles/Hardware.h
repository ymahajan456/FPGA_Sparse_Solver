#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "stdafx.h"

#ifdef DEFINE_VARIABLES
#define EXTERN                  
#define INITIALIZER(...)        = __VA_ARGS__
#else
#define EXTERN                  extern
#define INITIALIZER(...)       
#endif

#define HARDWARE_MAX_AU_OPERATIONS 10


/*
* Read Latency : valid output will be available at the corresponding port in the 
                    n-th cycle. e.g. read latency = 1 -> output will be available
                    in the next cycle
* Write latency: Output will be written to the corresponding memory cell by the 
                    end of the n-th cycle. e.g. write latency = 0 -> data will 
                    be written in that cycle itself
* Write First  : 
*/

struct BRAM
{
    int size;
    int ports;
    int readLatency;
    int writeLatency;
    bool WRITE_FIRST;
    // BRAM(int size, int ports, int readLatency);
};
/* ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠
* Only four port BRAMs are supported by the hardware generator
*/
EXTERN struct BRAM Xilinx_DualPort_BRAM INITIALIZER({1024, 4, 2, 0, true});
EXTERN struct BRAM Xilinx_SinglePort_BRAM INITIALIZER({2048, 1, 1, 0, true});

struct ArithmeticUnit
{
    int numOperations;
    string operations[HARDWARE_MAX_AU_OPERATIONS];
    bool pipelined;
    int pipeLatency;
    int delays[HARDWARE_MAX_AU_OPERATIONS];

    // ArithmeticUnit(int numOperations,const string operations[],int pipeLatency);
    // ArithmeticUnit(int numOperations, const string operations[], const int delays[]);
};

// struct ArithmeticUnit Xilinx_Floating_MAC_Unit();
EXTERN struct ArithmeticUnit Xilinx_Floating_MAC_Unit INITIALIZER({
    5,
    {"*","mac_add", "mac_sub", "+", "-"},
    true,
    3,
    {20,20,20,20,20}
});

EXTERN struct ArithmeticUnit Xilinx_Floating_Divide_Unit INITIALIZER({
    1,
    {"/"},
    true,
    3,
    {29}
});

EXTERN struct ArithmeticUnit Xilinx_Floating_AddSub_Unit INITIALIZER({
    2,
    {"+", "-"},
    true,
    12,
    {12,12}
});

EXTERN struct ArithmeticUnit Xilinx_Floating_Mul_Unit INITIALIZER({
    1,
    {"*"},
    true,
    9,
    {9}
});

EXTERN struct ArithmeticUnit Xilinx_Floating_ACC_Unit INITIALIZER({
    2,
    {"acc_add", "acc_sub"},
    true,
    24,
    {24,24}
});

/* ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠ ⚠
* IMPORTANT NOTICE : 
* 1: Arithmetic units must follow the order : MAC units and then Divider units
* 2: All BRAMs must have same number of ports
* 3: Notice 1 & 2 are due to poor handeling in the instruction conversion function. 
        Scheduling algorithm can handle the violations (Khatam karna hai; Just being lazy)
*/

struct HardwareConfig
{
    vector<BRAM *> BRAMs;
    vector<ArithmeticUnit *> AUs;
    vector<string> bramNames;
    int BRAMAddressWidth;
    int AUMuxSelWidth;
    int BRAMMuxSelWidth;
    int numMACUnits;
    int registers;
};

void exportHardwareConfig(struct HardwareConfig &hwConfig, const string configFileName = "../outputFiles/hwConfig.json" );


#endif
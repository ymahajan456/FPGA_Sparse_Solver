#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#include "stdafx.h"
#include "Graph.h"
#include "CCS.h"
#include "Hardware.h"

#ifdef DEFINE_VARIABLES
#define EXTERN                  
#define INITIALIZER(...)        = __VA_ARGS__
#else
#define EXTERN                  extern
#define INITIALIZER(...)       
#endif

#define SELECTION_TABLE_SIZE 200
#define MAX_BRAMS 16

struct BRAM_Operation
{
    int op = BRAM_NOTHING;             // read/ write/ NO_OP
    int BRAMIndex;
    int portIndex;
    int addr;
    int dinLoc;         // Live variable location
    int nodeId;         // node variable id
    Node *nodePtr;
};

struct AU_Operation
{
    string op = "NO_OP";
    int AUIndex;
    Node * nodePtr;
    vector<int> inputIds;
    vector<int> inputLocs;
    int assignClock, retireClock;
    int BRAMIndex;
    int BRAMAddress;
    int outputId;
};





// EXTERN struct BRAM_Operation NO_BRAM_OP INITIALIZER({
//     BRAM_NOTHING
// });


// EXTERN struct AU_Operation NO_AU_OP INITIALIZER({
//     "NO_OP"
// });


struct Schedule
{
    vector<vector<AU_Operation>> AUSchedule;
    vector<vector<BRAM_Operation>> memSchedule;
    void print();
};

void assignPriority(Graph &lGraph);
void assignAddresses(Graph &lGraph, CCS & A, CCS & L, CCS & U, HardwareConfig & hwConfig);
void assignScatteredAddresses(Graph &lGraph, CCS & A, CCS & L, CCS & U, HardwareConfig & hwConfig);

int  updateDoneStatus(vector<Node *> & retiringNodes, vector<Node *> & mayBeReadyNodes);
void findReadyNodes(vector<Node *> & readyNodes, vector<Node *> & mayBeReadyNodes, vector<Node *> previousCycle, vector<int> liveVariables);
void updateDirtyMemoryStatus(vector<Node *> completedWrites, vector<Node *> currCycle);
void scheduleNoReg(Graph & lGraph, HardwareConfig & hwConfig, Schedule & schedule);

void printReadableSchedule(const struct Schedule sch);

void convertScheduleToInstructionStream(const struct HardwareConfig hwConfig, const struct Schedule sch, const string instructionFileName = "../outputFiles/instructionStream.coe");

#endif

